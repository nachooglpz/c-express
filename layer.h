#ifndef LAYER_H
#define LAYER_H

#include "request.h"

typedef void (*Handler)(int client_fd, void (*next)(void *), void *context);

// Forward declaration for Router
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
} Layer;

int layer_match(Layer *layer, const char *method, const char *path);
int path_matches_pattern(const char *pattern, const char *path);

#endif