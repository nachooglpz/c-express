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

    // Handle mounted routers
    if (layer->type == LAYER_ROUTER && layer->mount_prefix) {
        // Check if path starts with mount prefix
        size_t prefix_len = strlen(layer->mount_prefix);
        if (strncmp(path, layer->mount_prefix, prefix_len) == 0) {
            // Path matches prefix, this router should handle it
            printf("[DEBUG] layer_match: router match=1 for method=%s, path=%s, mount_prefix=%s\n",
                   method, path, layer->mount_prefix);
            return 1;
        } else {
            printf("[DEBUG] layer_match: router match=0 for method=%s, path=%s, mount_prefix=%s\n",
                   method, path, layer->mount_prefix);
            return 0;
        }
    }

    // Handle regular handlers
    int method_match = strcmp(layer->method, method) == 0;
    int path_match = path_matches_pattern(layer->path, path);
    int match = method_match && path_match;
    
    printf("[DEBUG] layer_match: route match=%d for method=%s, path=%s, layer_method=%s, layer_path=%s\n",
           match, method, path, layer->method, layer->path);
    
    return match;
}