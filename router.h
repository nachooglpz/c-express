#ifndef ROUTER_H
#define ROUTER_H

#include "route.h"


typedef struct {
    Route *routes;
    int route_count;
    int capacity;
} Router;

void router_add_route(Router *router, const char *path, void (*handler)(int));
void router_handle(Router *router, const char *path, int client_fd);

#endif