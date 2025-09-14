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

void router_add_layer(Router *router, const char *method, const char *path, void (*handler)(int));
void router_handle(Router *router, const char *method, const char *path, int client_fd);

#endif