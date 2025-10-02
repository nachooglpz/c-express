#define _GNU_SOURCE
#include "router.h"
#include "../debug.h"
#include "layer.h"
#include "../http/response.h"
#include "../http/error.h"
#include "route.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

Router *create_router() {
    Router *router = malloc(sizeof(Router));
    if (!router) return NULL;
    
    router->layers = NULL;
    router->layer_count = 0;
    router->capacity = 0;
    
    // Set method pointers
    router->get = router_get;
    router->post = router_post;
    router->put = router_put;
    router->delete = router_delete;
    router->patch = router_patch;
    router->options = router_options;
    router->use = router_use;
    
    DEBUG_PRINT_STR("create_router: new router created\n");
    return router;
}

void destroy_router(Router *router) {
    if (router) {
        if (router->layers) {
            // Free mount_prefix strings for mounted routers
            for (int i = 0; i < router->layer_count; i++) {
                if (router->layers[i].mount_prefix) {
                    free((char*)router->layers[i].mount_prefix);
                }
            }
            free(router->layers);
        }
        free(router);
        DEBUG_PRINT_STR("destroy_router: router destroyed\n");
    }
}

void router_add_layer(Router *router, const char *method, const char *path, Handler handler) {
    if (router->layer_count >= router->capacity) {
        int new_capacity = router->capacity == 0 ? 4 : router->capacity * 2;
        Layer *new_layers = realloc(router->layers, new_capacity * sizeof(Layer));
        if (!new_layers) {
            // allocation failed
            return;
        }
        router->layers = new_layers;
        router->capacity = new_capacity;
    }

    Layer *layer = &router->layers[router->layer_count];
    layer->method = method;
    layer->path = path;
    layer->type = LAYER_HANDLER;
    layer->data.handler = handler;
    layer->mount_prefix = NULL;
    layer->last_match = NULL;
    
    // Compile route pattern for advanced matching
    if (path && (strchr(path, ':') || strchr(path, '*'))) {
        layer->pattern = (void*)compile_route_pattern(path);
        DEBUG_PRINT("router_add_layer: compiled pattern for '%s'\n", path);
    } else {
        layer->pattern = NULL;
    }
    
    router->layer_count++;
}

void router_mount(Router *parent, const char *prefix, Router *child) {
    if (parent->layer_count >= parent->capacity) {
        int new_capacity = parent->capacity == 0 ? 4 : parent->capacity * 2;
        Layer *new_layers = realloc(parent->layers, new_capacity * sizeof(Layer));
        if (!new_layers) {
            return;
        }
        parent->layers = new_layers;
        parent->capacity = new_capacity;
    }

    // Create a layer that represents the mounted router
    Layer *layer = &parent->layers[parent->layer_count];
    layer->method = "MOUNT";  // Special method for mounted routers
    layer->path = prefix;
    layer->type = LAYER_ROUTER;
    layer->data.router = child;
    layer->mount_prefix = strdup(prefix);  // Store a copy of the prefix
    parent->layer_count++;
    
    DEBUG_PRINT("router_mount: mounted router at prefix=%s\n", prefix);
}

void next_handler(void *context) {
    NextContext *ctx = (NextContext *)context;
    DEBUG_PRINT("next_handler: idx=%d, match_count=%d\n", ctx->idx, ctx->match_count);
    if (ctx->idx < ctx->match_count) {
        int layer_idx = ctx->matches[ctx->idx++];
        Layer *layer = &ctx->router->layers[layer_idx];
        
        if (layer->type == LAYER_ROUTER) {
            // Handle mounted router
            DEBUG_PRINT("next_handler: routing to mounted router at prefix=%s\n", layer->mount_prefix);
            
            // Strip the mount prefix from the path
            const char *sub_path = ctx->req->path;
            size_t prefix_len = strlen(layer->mount_prefix);
            if (strncmp(sub_path, layer->mount_prefix, prefix_len) == 0) {
                sub_path += prefix_len;
                // If sub_path is empty, make it "/"
                if (*sub_path == '\0') {
                    sub_path = "/";
                }
            }
            
            DEBUG_PRINT("next_handler: sub_path=%s for mounted router\n", sub_path);
            
            // Route to the mounted router with the stripped path
            router_handle(layer->data.router, ctx->req->method, sub_path, ctx->client_fd, ctx->req);
            
        } else {
            // Handle regular handler
            
            // Extract parameters from advanced pattern matching
            RouteMatch *route_match = (RouteMatch*)layer->last_match;
            if (route_match && route_match->matched) {
                request_set_route_params(ctx->req, route_match);
            } else if (layer->method && strcmp(layer->method, "USE") != 0 && layer->path) {
                // Fallback to legacy parameter parsing for simple patterns
                request_parse_params(ctx->req, layer->path, ctx->req->path);
            }
            
            DEBUG_PRINT("next_handler: calling handler for layer_idx=%d\n", layer_idx);
            layer->data.handler(ctx->client_fd, next_handler, ctx);
        }
    } else {
        DEBUG_PRINT_STR("next_handler: end of chain\n");
    }
}

void router_handle(Router *router, const char *method, const char *path, int client_fd, Request *req) {
    DEBUG_PRINT("router_handle: method=%s, path=%s\n", method, path);
    int *matches = malloc(router->layer_count * sizeof(int));
    int match_count = 0;
    int route_match_count = 0;  // Count only non-middleware matches
    
    for (int i = 0; i < router->layer_count; i++) {
        if (layer_match(&router->layers[i], method, path)) {
            matches[match_count++] = i;
            // Count non-middleware matches (routes that aren't "USE")
            if (strcmp(router->layers[i].method, "USE") != 0) {
                route_match_count++;
            }
        }
    }
    DEBUG_PRINT("router_handle: match_count=%d, route_match_count=%d\n", match_count, route_match_count);

    NextContext ctx = { router, NULL, matches, match_count, client_fd, 0, req, &route_match_count, NULL };

    if (match_count > 0) {
        next_handler(&ctx);
        
        // If we only had middleware matches but no route matches, send 404
        if (route_match_count == 0) {
            Response *res = create_response(client_fd);
            response_set_header(res, "Content-Type", "application/json");
            response_status(res, 404);
            response_send(res, "{\"error\":\"Not Found\",\"message\":\"The requested endpoint was not found\"}");
            destroy_response(res);
        }
    } else {
        // No matching routes found - send 404
        Response *res = create_response(client_fd);
        response_set_header(res, "Content-Type", "application/json");
        response_status(res, 404);
        response_send(res, "{\"error\":\"Not Found\",\"message\":\"The requested endpoint was not found\"}");
        destroy_response(res);
    }
    free(matches);
}

// Router method implementations for independent routing
void router_get(Router *router, const char *path, Handler handler) {
    router_add_layer(router, "GET", path, handler);
}

void router_post(Router *router, const char *path, Handler handler) {
    router_add_layer(router, "POST", path, handler);
}

void router_put(Router *router, const char *path, Handler handler) {
    router_add_layer(router, "PUT", path, handler);
}

void router_delete(Router *router, const char *path, Handler handler) {
    router_add_layer(router, "DELETE", path, handler);
}

void router_patch(Router *router, const char *path, Handler handler) {
    router_add_layer(router, "PATCH", path, handler);
}

void router_options(Router *router, const char *path, Handler handler) {
    router_add_layer(router, "OPTIONS", path, handler);
}

void router_use(Router *router, const char *path, Handler handler) {
    const char *use_path = path ? path : "/";
    router_add_layer(router, "USE", use_path, handler);
}