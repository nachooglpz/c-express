#include "layer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Helper function to get next segment from a path
const char* get_next_segment(const char *path, int *pos, char *buffer, int buffer_size) {
    if (!path || *pos >= strlen(path)) return NULL;
    
    // Skip slashes
    while (path[*pos] == '/') (*pos)++;
    if (*pos >= strlen(path)) return NULL;
    
    int start = *pos;
    // Find end of segment
    while (*pos < strlen(path) && path[*pos] != '/') (*pos)++;
    
    int length = *pos - start;
    if (length >= buffer_size) length = buffer_size - 1;
    
    strncpy(buffer, path + start, length);
    buffer[length] = '\0';
    
    return buffer;
}

// Check if a path matches a pattern (supports :param syntax)
int path_matches_pattern(const char *pattern, const char *path) {
    if (!pattern || !path) return 0;
    
    // Exact match for simple paths
    if (strchr(pattern, ':') == NULL) {
        return strcmp(pattern, path) == 0;
    }
    
    // Pattern matching for parameterized routes
    int pattern_pos = 0, path_pos = 0;
    char pattern_segment[64], path_segment[64];
    
    while (1) {
        const char *p_seg = get_next_segment(pattern, &pattern_pos, pattern_segment, sizeof(pattern_segment));
        const char *path_seg = get_next_segment(path, &path_pos, path_segment, sizeof(path_segment));
        
        // Both exhausted - perfect match
        if (!p_seg && !path_seg) {
            return 1;
        }
        
        // One exhausted but not the other - no match
        if (!p_seg || !path_seg) {
            return 0;
        }
        
        // Compare segments (parameter segments always match)
        if (p_seg[0] != ':' && strcmp(p_seg, path_seg) != 0) {
            return 0;
        }
    }
}

int layer_match(Layer *layer, const char *method, const char *path) {
    if (strcmp(layer->method, "USE") == 0) {
        printf("[DEBUG] layer_match: middleware matched for method=%s, path=%s\n", method, path);
        return 1; // always match for middleware
    }

    int method_match = strcmp(layer->method, method) == 0;
    int path_match = path_matches_pattern(layer->path, path);
    int match = method_match && path_match;
    
    printf("[DEBUG] layer_match: route match=%d for method=%s, path=%s, layer_method=%s, layer_path=%s\n",
           match, method, path, layer->method, layer->path);
    
    return match;
}