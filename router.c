#include "router.h"
#include <string.h>
#include <stdlib.h>

void router_add_route(Router *router, const char *path, void (*handler)(int)) {
    if (router->route_count >= router->capacity) {
        int new_capacity = router->capacity == 0 ? 4 : router->capacity * 2;
        Route *new_routes = realloc(router->routes, new_capacity * sizeof(Route));
        if (!new_routes) {
            // Allocation failed
            return;
        }
        router->routes = new_routes;
        router->capacity = new_capacity;
    }
    router->routes[router->route_count].path = path;
    router->routes[router->route_count].layer.path = path;
    router->routes[router->route_count].layer.handler = handler;
    router->route_count++;
}

void router_handle(Router * router, const char *path, int client_fd) {
    for (int i = 0; i < router->route_count; i++) {
        if (strcmp(router->routes[i].path, path) == 0) {
            router->routes[i].layer.handler(client_fd);
            return;
        }
    }

    // no route matched
}