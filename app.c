#include "app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stddef.h>

// Default error handler
void default_error_handler(Error *error, int client_fd, void *context) {
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
    
    next(ctx);
    
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

        char buffer[1024] = {0};
        read(client_fd, buffer, sizeof(buffer) -1);
        printf("[DEBUG] app_listen: received request: %s\n", buffer);

        // Create and initialize request object
        Request *req = malloc(sizeof(Request));
        request_init(req, client_fd, buffer);

        app_handle_request(app, req->method, req->path, client_fd, req);
        
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
    app.error = app_error;

    // automatically register express_init middleware
    app.use(&app, express_init);
    return app;
}