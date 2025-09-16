#include "router.h"
#include "layer.h"
#include <string.h>
#include <stdlib.h>

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

static void next_handler(void *context) {
    NextContext *ctx = (NextContext *)context;
    if (ctx->idx < ctx->match_count) {
        int layer_idx = ctx->matches[ctx->idx++];
        ctx->router->layers[layer_idx].handler(ctx->client_fd, next_handler, ctx);
    }
}

void router_handle(Router *router, const char *method, const char *path, int client_fd) {
    int *matches = malloc(router->layer_count * sizeof(int));
    int match_count = 0;
    for (int i = 0; i < router->layer_count; i++) {
        if (layer_match(&router->layers[i], method, path)) {
            matches[match_count++] = i;
        }
    }

    NextContext ctx = { router, matches, match_count, client_fd, 0 };

    if (match_count > 0) {
        next_handler(&ctx);
    } else {
        const char *response =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 9\r\n"
            "\r\n"
            "Not Found";
        write(client_fd, response, strlen(response));
    }
    free(matches);
}

void router_use(Router *router, Handler handler) {
    router_add_layer(router, "USE", "/", handler);
}