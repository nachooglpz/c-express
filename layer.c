#include "layer.h"
#include <string.h>

int layer_match(Layer *layer, const char *method, const char *path) {
    if (strcmp(layer->method, "USE") == 0) {
        return 1; // always match for middleware
        printf("[DEBUG] layer_match: middleware matched for method=%s, path=%s\n", method, path);
    }

    int match = strcmp(layer->method, method) == 0 && strcmp(layer->path, path) == 0;
    printf("[DEBUG] layer_match: route match=%d for method=%s, path=%s, layer_method=%s, layer_path=%s\n",
           match, method, path, layer->method, layer->path);
    
    return match;
}