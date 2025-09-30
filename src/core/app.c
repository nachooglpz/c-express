#define _GNU_SOURCE
#include "app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stddef.h>

// Default error handler
void default_error_handler(Error *error, int client_fd, void *context) {
    (void)context; // Suppress unused parameter warning
    Response *res = create_response(client_fd);
    
    // Set appropriate status code
    response_status(res, error->status_code);
    response_set_header(res, "Content-Type", "application/json");
    
    // Create error response
    char error_response[1024];
    snprintf(error_response, sizeof(error_response),
        "{"
        "\"error\":\"%s\","
        "\"message\":\"%s\","
        "\"status\":%d,"
        "\"timestamp\":\"%s\""
        "}",
        error_get_status_text(error->code),
        error->message,
        error->status_code,
        "2025-09-23T00:00:00Z"  // Could be enhanced with actual timestamp
    );
    
    response_send(res, error_response);
    destroy_response(res);
    
    // Mark error as handled
    error->is_handled = true;
}

// middleware to attach Response and ErrorContext to each request
void express_init(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = malloc(sizeof(Response));
    ErrorContext *error_ctx = create_error_context();
    
    response_init(res, client_fd);
    ctx->user_context = res;
    ctx->error_ctx = error_ctx;
    
    printf("[DEBUG] express_init: initialized request context\n");
    
    // Check if request is using streaming
    if (ctx->req && ctx->req->is_body_streamed && ctx->req->is_body_streamed(ctx->req)) {
        printf("[DEBUG] express_init: request is using streaming mode\n");
        
        StreamContext *stream = ctx->req->get_stream(ctx->req);
        if (stream) {
            printf("[DEBUG] express_init: stream context available, bytes read: %zu\n", 
                   stream_get_content_length(stream));
            
            if (stream_has_error(stream)) {
                printf("[DEBUG] express_init: stream has error: %s\n", stream_get_error(stream));
            }
        }
    } else {
        printf("[DEBUG] express_init: request is using legacy mode\n");
    }
    
    next(ctx);
    
    printf("[DEBUG] express_init: middleware chain completed\n");
    
    // Clean up JSON and form resources from request if they were used
    if (ctx->req) {
        request_free_json(ctx->req);
        request_free_form(ctx->req);
        
        // Clean up streaming resources
        if (ctx->req->stream) {
            printf("[DEBUG] express_init: cleaning up streaming resources\n");
            request_free_stream(ctx->req);
        }
        
        printf("[DEBUG] express_init: cleaned up JSON and form data resources\n");
    }
    
    // Check for unhandled errors after middleware chain
    if (error_context_has_error(error_ctx) && !error_ctx->current_error->is_handled) {
        printf("[ERROR] Unhandled error caught: %s\n", error_ctx->current_error->message);
        
        // Use custom error handler if available
        if (ctx->app && ctx->app->error_handler) {
            ctx->app->error_handler(error_ctx->current_error, client_fd, ctx);
        } else {
            default_error_handler(error_ctx->current_error, client_fd, ctx);
        }
    }
    
    destroy_error_context(error_ctx);
    free(res);
    printf("[DEBUG] express_init: cleanup completed\n");
}

void app_get(App *app, const char *path, Handler handler) {
    printf("[DEBUG] app_get: path=%s\n", path);
    router_add_layer(&app->router, "GET", path, handler);
}

void app_post(App *app, const char *path, Handler handler) {
    printf("[DEBUG] app_post: path=%s\n", path);
    router_add_layer(&app->router, "POST", path, handler);
}

void app_put(App *app, const char *path, Handler handler) {
    printf("[DEBUG] app_put: path=%s\n", path);
    router_add_layer(&app->router, "PUT", path, handler);
}

void app_delete(App *app, const char *path, Handler handler) {
    printf("[DEBUG] app_delete: path=%s\n", path);
    router_add_layer(&app->router, "DELETE", path, handler);
}

void app_patch(App *app, const char *path, Handler handler) {
    printf("[DEBUG] app_patch: path=%s\n", path);
    router_add_layer(&app->router, "PATCH", path, handler);
}

void app_options(App *app, const char *path, Handler handler) {
    printf("[DEBUG] app_options: path=%s\n", path);
    router_add_layer(&app->router, "OPTIONS", path, handler);
}

void app_use(App *app, Handler handler) {
    printf("[DEBUG] app_use: registering middleware\n");
    router_add_layer(&app->router, "USE", "/", handler);
}

void app_mount(App *app, const char *prefix, Router *router) {
    printf("[DEBUG] app_mount: mounting router at prefix=%s\n", prefix);
    router_mount(&app->router, prefix, router);
}

void app_error(App *app, ErrorHandler handler) {
    printf("[DEBUG] app_error: registering error handler\n");
    app->error_handler = handler;
}

void app_listen(App *app, int port) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // bind
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    // listen
    listen(server_fd, 3);
    printf("Listening on port %d\n", port);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        printf("[DEBUG] app_listen: accepted client_fd=%d\n", client_fd);

        char headers_buffer[8192] = {0}; // Buffer for HTTP headers only
        ssize_t headers_read = 0;
        ssize_t bytes_read;
        int headers_complete = 0;
        
        // First, read only the headers (up to \r\n\r\n)
        while (headers_read < (ssize_t)(sizeof(headers_buffer) - 1) && !headers_complete) {
            bytes_read = read(client_fd, headers_buffer + headers_read, 1);
            if (bytes_read <= 0) break;
            
            headers_read += bytes_read;
            headers_buffer[headers_read] = '\0';
            
            // Check if we have complete headers
            if (strstr(headers_buffer, "\r\n\r\n")) {
                headers_complete = 1;
                break;
            }
        }
        
        if (!headers_complete) {
            printf("[DEBUG] app_listen: incomplete headers received\n");
            close(client_fd);
            continue;
        }
        
        printf("[DEBUG] app_listen: received headers (%zd bytes): %.500s\n", headers_read, headers_buffer);
        
        // Parse headers to determine if we need streaming
        const char *content_length_header = strstr(headers_buffer, "Content-Length: ");
        const char *transfer_encoding_header = strstr(headers_buffer, "Transfer-Encoding: ");
        
        int use_streaming = 0;
        int content_length = 0;
        
        // Determine if we should use streaming
        if (transfer_encoding_header && strstr(transfer_encoding_header, "chunked")) {
            use_streaming = 1;
            printf("[DEBUG] app_listen: chunked encoding detected, using streaming\n");
        } else if (content_length_header) {
            content_length = atoi(content_length_header + 16);
            printf("[DEBUG] app_listen: Content-Length: %d\n", content_length);
            
            if (content_length > MAX_BODY_SIZE) {
                use_streaming = 1;
                printf("[DEBUG] app_listen: large body detected (%d bytes), using streaming\n", content_length);
            }
        }
        
        // Create and initialize request object
        Request *req = malloc(sizeof(Request));
        
        if (use_streaming) {
            // Use streaming initialization
            printf("[DEBUG] app_listen: initializing request with streaming\n");
            request_init_streaming(req, client_fd, headers_buffer);
            
            // Stream context is set up and ready for on-demand reading
            if (req->stream && !stream_has_error(req->stream)) {
                printf("[DEBUG] app_listen: streaming context ready for on-demand reading\n");
                // Don't read data automatically - let the application decide when to stream
                req->body_complete = 0; // Mark as not complete since we haven't read it yet
            } else if (req->stream) {
                printf("[DEBUG] app_listen: streaming setup failed: %s\n", stream_get_error(req->stream));
            }
        } else {
            // Use legacy initialization for small bodies
            printf("[DEBUG] app_listen: using legacy body handling\n");
            
            char complete_request[16384] = {0};
            size_t total_size = headers_read;
            
            // Copy headers to complete request buffer
            memcpy(complete_request, headers_buffer, headers_read);
            
            // Read body if there's a content length
            if (content_length > 0 && content_length < (int)(sizeof(complete_request) - headers_read - 1)) {
                ssize_t body_read = read(client_fd, complete_request + headers_read, content_length);
                if (body_read > 0) {
                    total_size += body_read;
                    complete_request[total_size] = '\0';
                    printf("[DEBUG] app_listen: read body (%zd bytes) for legacy processing\n", body_read);
                }
            }
            
            printf("[DEBUG] app_listen: complete request (%zu bytes): %.500s\n", total_size, complete_request);
            request_init(req, client_fd, complete_request);
        }

        // Handle the request
        app_handle_request(app, req->method, req->path, client_fd, req);
        
        // Comprehensive cleanup - free all allocated memory in request
        request_destroy(req);
        free(req);
        close(client_fd);
    }
}

void app_handle_request(App *app, const char *method, const char *path, int client_fd, Request *req) {
    Router *router = &app->router;
    int *matches = malloc(router->layer_count * sizeof(int));
    int match_count = 0;
    int route_match_count = 0;
    
    for (int i = 0; i < router->layer_count; i++) {
        if (layer_match(&router->layers[i], method, path)) {
            matches[match_count++] = i;
            if (strcmp(router->layers[i].method, "USE") != 0) {
                route_match_count++;
            }
        }
    }
    
    NextContext ctx = { router, app, matches, match_count, client_fd, 0, req, &route_match_count, NULL };
    
    if (match_count > 0) {
        next_handler(&ctx);
        
        if (route_match_count == 0) {
            Response *res = create_response(client_fd);
            response_set_header(res, "Content-Type", "application/json");
            response_status(res, 404);
            response_send(res, "{\"error\":\"Not Found\",\"message\":\"The requested endpoint was not found\"}");
            destroy_response(res);
        }
    } else {
        Response *res = create_response(client_fd);
        response_set_header(res, "Content-Type", "application/json");
        response_status(res, 404);
        response_send(res, "{\"error\":\"Not Found\",\"message\":\"The requested endpoint was not found\"}");
        destroy_response(res);
    }
    
    free(matches);
}

App create_app() {
    App app;
    app.router.layers = NULL;
    app.router.layer_count = 0;
    app.router.capacity = 0;
    app.error_handler = NULL;  // Initialize error handler
    app.get = app_get;
    app.post = app_post;
    app.put = app_put;
    app.delete = app_delete;
    app.patch = app_patch;
    app.options = app_options;
    app.listen = app_listen;
    app.use = app_use;
    app.mount = app_mount;
    app.error = app_error;

    // automatically register express_init middleware
    app.use(&app, express_init);
    return app;
}

// ============================================================================
// ENHANCED ROUTE REGISTRATION WITH METADATA
// ============================================================================

// Helper function to apply route configuration
void apply_route_config(Route *route, const RouteConfig *config) {
    if (!route || !config) return;
    
    // Set route ID
    if (config->route_id) {
        route->route_id = strdup(config->route_id);
    }
    
    // Create metadata if not exists
    if (!route->metadata) {
        route->metadata = create_route_metadata();
    }
    
    RouteMetadata *meta = route->metadata;
    
    // Set basic information
    if (config->summary) {
        set_route_summary(meta, config->summary);
    }
    
    if (config->description) {
        set_route_description(meta, config->description);
    }
    
    // Add tags
    if (config->tags) {
        for (int i = 0; config->tags[i]; i++) {
            add_route_tag(meta, config->tags[i]);
        }
    }
    
    // Set deprecated status
    if (config->deprecated) {
        const char *reason = config->deprecated_reason ? 
                           config->deprecated_reason : 
                           "This route is deprecated";
        set_route_deprecated(meta, reason);
    }
}

// Enhanced GET route registration
void app_get_with_metadata(App *app, const char *path, Handler handler, const RouteConfig *config) {
    if (!app || !path || !handler) return;
    
    // Register the route normally first
    app_get(app, path, handler);
    
    // Find the last added route and apply metadata
    // Note: This is a simple approach - in a more robust implementation,
    // we might want to return the route from the registration functions
    if (config) {
        // For now, we'll need to access the router's layers
        // This would need router API enhancement to get the last added route
        printf("[DEBUG] app_get_with_metadata: registered route %s with metadata\n", path);
        if (config->route_id) {
            printf("[DEBUG] Route ID: %s\n", config->route_id);
        }
        if (config->summary) {
            printf("[DEBUG] Summary: %s\n", config->summary);
        }
    }
}

// Enhanced POST route registration
void app_post_with_metadata(App *app, const char *path, Handler handler, const RouteConfig *config) {
    if (!app || !path || !handler) return;
    
    app_post(app, path, handler);
    
    if (config) {
        printf("[DEBUG] app_post_with_metadata: registered route %s with metadata\n", path);
        if (config->route_id) {
            printf("[DEBUG] Route ID: %s\n", config->route_id);
        }
    }
}

// Enhanced PUT route registration
void app_put_with_metadata(App *app, const char *path, Handler handler, const RouteConfig *config) {
    if (!app || !path || !handler) return;
    
    app_put(app, path, handler);
    
    if (config) {
        printf("[DEBUG] app_put_with_metadata: registered route %s with metadata\n", path);
    }
}

// Enhanced DELETE route registration
void app_delete_with_metadata(App *app, const char *path, Handler handler, const RouteConfig *config) {
    if (!app || !path || !handler) return;
    
    app_delete(app, path, handler);
    
    if (config) {
        printf("[DEBUG] app_delete_with_metadata: registered route %s with metadata\n", path);
    }
}

// Enhanced PATCH route registration
void app_patch_with_metadata(App *app, const char *path, Handler handler, const RouteConfig *config) {
    if (!app || !path || !handler) return;
    
    app_patch(app, path, handler);
    
    if (config) {
        printf("[DEBUG] app_patch_with_metadata: registered route %s with metadata\n", path);
    }
}

// Enhanced OPTIONS route registration
void app_options_with_metadata(App *app, const char *path, Handler handler, const RouteConfig *config) {
    if (!app || !path || !handler) return;
    
    app_options(app, path, handler);
    
    if (config) {
        printf("[DEBUG] app_options_with_metadata: registered route %s with metadata\n", path);
    }
}

// Print all routes with their metadata
void app_print_routes(App *app) {
    if (!app) return;
    
    printf("\n=== APPLICATION ROUTES ===\n");
    printf("Note: Full route introspection requires router API enhancement\n");
    printf("For now, showing debug information from route registration\n");
    printf("==========================\n\n");
}

// Generate OpenAPI documentation for all routes
char* app_generate_openapi_doc(App *app) {
    if (!app) return NULL;
    
    // This would need router API enhancement to access all routes
    char *doc = malloc(512);
    if (!doc) return NULL;
    
    strcpy(doc, "{\n"
               "  \"openapi\": \"3.0.0\",\n"
               "  \"info\": {\n"
               "    \"title\": \"C-Express API\",\n"
               "    \"version\": \"1.0.0\"\n"
               "  },\n"
               "  \"paths\": {\n"
               "    \"Note\": \"Full OpenAPI generation requires router enhancement\"\n"
               "  }\n"
               "}");
    
    return doc;
}

// Find routes by tag
Route** app_get_routes_by_tag(App *app, const char *tag, int *count) {
    if (!app || !tag || !count) return NULL;
    
    *count = 0;
    printf("[DEBUG] app_get_routes_by_tag: searching for tag '%s'\n", tag);
    printf("[DEBUG] Note: Full implementation requires router API enhancement\n");
    
    return NULL;
}

// Find route by ID
Route* app_get_route_by_id(App *app, const char *route_id) {
    if (!app || !route_id) return NULL;
    
    printf("[DEBUG] app_get_route_by_id: searching for route '%s'\n", route_id);
    printf("[DEBUG] Note: Full implementation requires router API enhancement\n");
    
    return NULL;
}