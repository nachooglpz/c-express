#ifndef LAYER_H
#define LAYER_H

typedef struct {
    const char *path;
    const char *method;
    void (*handler)(int client_fd);
} Layer;

int layer_match(Layer *layer, const char *method, const char *path);

#endif