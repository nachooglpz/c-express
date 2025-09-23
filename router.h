#ifndef ROUTER_H
#define ROUTER_H

#include "route.h"
#include "request.h"
#include "error.h"

typedef struct {
    // Route *routes;
    Layer *layers;
    // int route_count;
    int layer_count;
    int capacity;
} Router;

// Forward declaration for App
struct App;

// context for next middleware
typedef struct {
    Router *router;
    struct App *app;     // Reference to the app for error handling
    int *matches;
    int match_count;
    int client_fd;
    int idx;
    Request *req;        // Request object
    void *user_context; // holds Response* or other user context
    ErrorContext *error_ctx; // Error handling context
} NextContext;

void router_add_layer(Router *router, const char *method, const char *path, Handler handler);
void router_handle(Router *router, const char *method, const char *path, int client_fd, Request *req);
void router_use(Router *router, Handler handler);
void next_handler(void *context);

#endif