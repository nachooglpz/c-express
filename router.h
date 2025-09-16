#ifndef ROUTER_H
#define ROUTER_H

#include "route.h"


typedef struct {
    // Route *routes;
    Layer *layers;
    // int route_count;
    int layer_count;
    int capacity;
} Router;

// context for next middleware
typedef struct {
    Router *router;
    int *matches;
    int match_count;
    int client_fd;
    int idx;
    void *user_context; // holds Response* or other user context
} NextContext;

void router_add_layer(Router *router, const char *method, const char *path, Handler handler);
static void next_handler(void *context);
void router_handle(Router *router, const char *method, const char *path, int client_fd);
void router_use(Router *router, Handler handler);

#endif