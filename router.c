#include "router.h"
#include "layer.h"
#include "response.h"
#include "error.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

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

    router->layers[router->layer_count].method = method;
    router->layers[router->layer_count].path = path;
    router->layers[router->layer_count].handler = handler;
    router->layer_count++;
}

void next_handler(void *context) {
    NextContext *ctx = (NextContext *)context;
    printf("[DEBUG] next_handler: idx=%d, match_count=%d\n", ctx->idx, ctx->match_count);
    if (ctx->idx < ctx->match_count) {
        int layer_idx = ctx->matches[ctx->idx++];
        Layer *layer = &ctx->router->layers[layer_idx];
        
        // Parse URL parameters if this is a route (not middleware)
        if (strcmp(layer->method, "USE") != 0) {
            request_parse_params(ctx->req, layer->path, ctx->req->path);
        }
        
        printf("[DEBUG] next_handler: calling handler for layer_idx=%d\n", layer_idx);
        layer->handler(ctx->client_fd, next_handler, ctx);
    } else {
        printf("[DEBUG] next_handler: end of chain\n");
    }
}

void router_handle(Router *router, const char *method, const char *path, int client_fd, Request *req) {
    printf("[DEBUG] router_handle: method=%s, path=%s\n", method, path);
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
    printf("[DEBUG] router_handle: match_count=%d, route_match_count=%d\n", match_count, route_match_count);

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

void router_use(Router *router, Handler handler) {
    router_add_layer(router, "USE", "/", handler);
}