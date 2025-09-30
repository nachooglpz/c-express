#ifndef ROUTER_H
#define ROUTER_H

#include "route.h"
#include "../http/request.h"
#include "../http/error.h"

struct Router {
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
    void (*use)(struct Router *router, const char *path, Handler handler);
};

typedef struct Router Router;

// Forward declaration for App
struct App;

// context for next middleware
typedef struct {
    struct Router *router;
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
struct Router *create_router();
void destroy_router(struct Router *router);
void router_add_layer(struct Router *router, const char *method, const char *path, Handler handler);
void router_handle(struct Router *router, const char *method, const char *path, int client_fd, Request *req);
void router_use(struct Router *router, const char *path, Handler handler);
void router_mount(struct Router *parent, const char *prefix, struct Router *child);
void next_handler(void *context);

// Router method implementations
void router_get(struct Router *router, const char *path, Handler handler);
void router_post(struct Router *router, const char *path, Handler handler);
void router_put(struct Router *router, const char *path, Handler handler);
void router_delete(struct Router *router, const char *path, Handler handler);
void router_patch(struct Router *router, const char *path, Handler handler);
void router_options(struct Router *router, const char *path, Handler handler);

#endif