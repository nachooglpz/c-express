#include "layer.h"
#include "../debug.h"
#include "route.h"
#include "../debug.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Helper function to get next segment from a path
const char* get_next_segment(const char *path, int *pos, char *buffer, int buffer_size) {
    if (!path || *pos >= (int)strlen(path)) return NULL;
    
    // Skip slashes
    while (path[*pos] == '/') (*pos)++;
    if (*pos >= (int)strlen(path)) return NULL;
    
    int start = *pos;
    // Find end of segment
    while (*pos < (int)strlen(path) && path[*pos] != '/') (*pos)++;
    
    int length = *pos - start;
    if (length >= buffer_size) length = buffer_size - 1;
    
    strncpy(buffer, path + start, length);
    buffer[length] = '\0';
    
    return buffer;
}

// Legacy simple pattern matching (for backwards compatibility)
int path_matches_pattern(const char *pattern, const char *path) {
    if (!pattern || !path) return 0;
    
    // Exact match for simple paths
    if (strchr(pattern, ':') == NULL && strchr(pattern, '*') == NULL) {
        return strcmp(pattern, path) == 0;
    }
    
    // Use advanced pattern matching for complex patterns
    RoutePattern *compiled = compile_route_pattern(pattern);
    if (!compiled) return 0;
    
    RouteMatch match = route_pattern_match(compiled, path);
    int result = match.matched;
    
    free_route_match(&match);
    free_route_pattern(compiled);
    
    return result;
}

// Get the last match result from a layer (for parameter access)
void* layer_get_match_result(Layer *layer) {
    return layer ? layer->last_match : NULL;
}

int layer_match(Layer *layer, const char *method, const char *path) {
    // Clear previous match result
    if (layer->last_match) {
        free_route_match((RouteMatch*)layer->last_match);
        free(layer->last_match);
        layer->last_match = NULL;
    }
    
    if (layer->method && strcmp(layer->method, "USE") == 0) {
        DEBUG_PRINT("layer_match: middleware matched for method=%s, path=%s\n", method, path);
        
        // For middleware with patterns, still check the path
        if (layer->pattern) {
            RouteMatch match = route_pattern_match((RoutePattern*)layer->pattern, path);
            if (match.matched) {
                layer->last_match = duplicate_route_match(&match);
                free_route_match(&match);
                return 1;
            } else {
                free_route_match(&match);
                return 0;
            }
        }
        
        return 1; // Always match for middleware without patterns
    }

    // Handle mounted routers
    if (layer->type == LAYER_ROUTER && layer->mount_prefix) {
        // Check if path starts with mount prefix
        size_t prefix_len = strlen(layer->mount_prefix);
        if (strncmp(path, layer->mount_prefix, prefix_len) == 0) {
            // Path matches prefix, this router should handle it
            DEBUG_PRINT("layer_match: router match=1 for method=%s, path=%s, mount_prefix=%s\n",
                   method, path, layer->mount_prefix);
            return 1;
        } else {
            DEBUG_PRINT("layer_match: router match=0 for method=%s, path=%s, mount_prefix=%s\n",
                   method, path, layer->mount_prefix);
            return 0;
        }
    }

    // Handle regular handlers
    int method_match = layer->method ? strcmp(layer->method, method) == 0 : 1;
    int path_match = 0;
    
    if (layer->pattern) {
        // Use advanced pattern matching
        RouteMatch match = route_pattern_match((RoutePattern*)layer->pattern, path);
        path_match = match.matched;
        
        if (path_match) {
            // Validate constraints if this is a route handler (not middleware)
            if (layer->method && strcmp(layer->method, "USE") != 0) {
                ValidationError *errors = NULL;
                int error_count = 0;
                
                if (!validate_route_match(&match, &errors, &error_count)) {
                    DEBUG_PRINT("layer_match: constraint validation failed (%d errors)\n", error_count);
                    
                    // Free validation errors
                    for (int i = 0; i < error_count; i++) {
                        free_validation_error(&errors[i]);
                    }
                    free(errors);
                    
                    free_route_match(&match);
                    return 0; // Constraint validation failed
                }
            }
            
            // Store match result for parameter access
            layer->last_match = duplicate_route_match(&match);
            DEBUG_PRINT("layer_match: advanced pattern matched with %d parameters\n", match.param_count);
            free_route_match(&match);
        } else {
            free_route_match(&match);
        }
    } else if (layer->path) {
        // Use legacy pattern matching
        path_match = path_matches_pattern(layer->path, path);
    }
    
    int match = method_match && path_match;
    
    DEBUG_PRINT("layer_match: route match=%d for method=%s, path=%s, layer_method=%s, layer_path=%s\n",
           match, method, path, layer->method ? layer->method : "NULL", layer->path ? layer->path : "NULL");
    
    return match;
}