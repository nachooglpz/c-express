#ifndef ROUTER_H
#define ROUTER_H

#include "route.h"
#include "request.h"
#include "error.h"

typedef struct Router {
    Layer *layers;
    int layer_count;
    int capacity;
    
    // Router methods (similar to App)
    void (*get)(struct Router *, const char *path, Handler handler);
    void (*post)(struct Router *, const char *path, Handler handler);
    void (*put)(struct Router *, const char *path, Handler handler);
    void (*delete)(struct Router *, const char *path, Handler handler);
    void (*patch)(struct Router *, const char *path, Handler handler);
    void (*options)(struct Router *, const char *path, Handler handler);
    void (*use)(Router *router, const char *path, Handler handler);
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

// Router functions
Router *create_router();
void destroy_router(Router *router);
void router_add_layer(Router *router, const char *method, const char *path, Handler handler);
void router_handle(Router *router, const char *method, const char *path, int client_fd, Request *req);
void router_use(Router *router, const char *path, Handler handler);
void router_mount(Router *parent, const char *prefix, Router *child);
void next_handler(void *context);

// Router method implementations
void router_get(Router *router, const char *path, Handler handler);
void router_post(Router *router, const char *path, Handler handler);
void router_put(Router *router, const char *path, Handler handler);
void router_delete(Router *router, const char *path, Handler handler);
void router_patch(Router *router, const char *path, Handler handler);
void router_options(Router *router, const char *path, Handler handler);

#endif