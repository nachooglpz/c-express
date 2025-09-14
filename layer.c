#include "layer.h"
#include <string.h>

int layer_match(Layer *layer, const char *method, const char *path) {
    return strcmp(layer->method, method) == 0 && strcmp(layer->path, path) == 0;
}