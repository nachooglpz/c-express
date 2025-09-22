#ifndef ROUTER_H
#define ROUTER_H

#include "route.h"
#include "request.h"

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
    Request *req;        // Request object
    void *user_context; // holds Response* or other user context
} NextContext;

void router_add_layer(Router *router, const char *method, const char *path, Handler handler);
void router_handle(Router *router, const char *method, const char *path, int client_fd, Request *req);
void router_use(Router *router, Handler handler);

#endif