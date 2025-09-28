#include "route.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== Minimal Route Pattern Test ===\n");
    
    // Test basic pattern compilation
    printf("Testing pattern compilation...\n");
    RoutePattern *pattern = compile_route_pattern("/users/:id");
    
    if (!pattern) {
        printf("ERROR: Failed to compile pattern\n");
        return 1;
    }
    
    printf("Pattern compiled successfully!\n");
    printf("  Original: %s\n", pattern->original_pattern);
    printf("  Segments: %d\n", pattern->segment_count);
    printf("  Priority: %d\n", pattern->priority);
    
    // Test pattern matching
    printf("\nTesting pattern matching...\n");
    RouteMatch match = route_pattern_match(pattern, "/users/123");
    
    if (match.matched) {
        printf("Match successful!\n");
        printf("  Parameters: %d\n", match.param_count);
        for (int i = 0; i < match.param_count; i++) {
            printf("  Param[%d]: %s = %s\n", i, match.params[i].name, match.params[i].value);
        }
    } else {
        printf("Match failed\n");
    }
    
    // Cleanup
    free_route_match(&match);
    free_route_pattern(pattern);
    
    printf("Test completed successfully!\n");
    return 0;
}
