#ifndef LAYER_H
#define LAYER_H

#include "../http/request.h"

typedef void (*Handler)(int client_fd, void (*next)(void *), void *context);

// Forward declarations
typedef struct Router Router;

typedef enum {
    LAYER_HANDLER,    // Regular handler
    LAYER_ROUTER      // Mounted sub-router
} LayerType;

typedef struct {
    const char *path;
    const char *method;
    LayerType type;
    union {
        Handler handler;      // For LAYER_HANDLER
        Router *router;       // For LAYER_ROUTER  
    } data;
    const char *mount_prefix; // For mounted routers
    void *pattern;            // Compiled pattern for advanced matching (RoutePattern*)
    void *last_match;         // Store last match result for parameter access (RouteMatch*)
} Layer;

int layer_match(Layer *layer, const char *method, const char *path);
int path_matches_pattern(const char *pattern, const char *path);
void* layer_get_match_result(Layer *layer);

#endif