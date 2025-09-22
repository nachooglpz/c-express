#ifndef LAYER_H
#define LAYER_H

#include "request.h"

typedef void (*Handler)(int client_fd, void (*next)(void *), void *context);

typedef struct {
    const char *path;
    const char *method;
    Handler handler;
} Layer;

int layer_match(Layer *layer, const char *method, const char *path);
int path_matches_pattern(const char *pattern, const char *path);

#endif