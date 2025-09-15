#ifndef LAYER_H
#define LAYER_H

typedef void (*Handler)(int client_fd, void (*next)(void *), void *context);

typedef struct {
    const char *path;
    const char *method;
    Handler handler;
} Layer;

int layer_match(Layer *layer, const char *method, const char *path);

#endif