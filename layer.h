#ifndef LAYER_H
#define LAYER_H

typedef struct {
    const char *path;
    void (*handler)(int client_fd);
} Layer;

#endif