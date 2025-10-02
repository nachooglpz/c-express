#include <stdio.h>
#include <stdlib.h>
#include "../src/http/streaming.h"
#include "../src/core/router.h"
#include "../src/http/error.h"

int main() {
    printf("Testing Streaming, Router, and Error memory management...\n");
    
    // Test 1: Streaming module
    printf("\n=== STREAMING MODULE TEST ===\n");
    for (int i = 0; i < 3; i++) {
        printf("Creating stream context %d...\n", i+1);
        
        // Test different stream modes
        StreamContext *stream = NULL;
        if (i == 0) {
            // Small body - memory mode
            stream = stream_create(-1, "100", NULL);
        } else if (i == 1) {
            // No content length
            stream = stream_create(-1, NULL, NULL);
        } else {
            // Chunked mode
            stream = stream_create(-1, NULL, "chunked");
        }
        
        if (stream) {
            printf("Stream %d created successfully (mode: %d)\n", i+1, stream->mode);
            stream_destroy(stream);
            printf("Stream %d destroyed\n", i+1);
        } else {
            printf("Failed to create stream %d\n", i+1);
        }
    }
    
    // Test 2: Router module
    printf("\n=== ROUTER MODULE TEST ===\n");
    for (int i = 0; i < 3; i++) {
        printf("Creating router %d...\n", i+1);
        
        Router *router = create_router();
        if (router) {
            printf("Router %d created successfully\n", i+1);
            
            // Test router mounting (this uses strdup)
            Router *child = create_router();
            if (child) {
                router_mount(router, "/api", child);
                printf("Child router mounted at /api\n");
                destroy_router(child);
            }
            
            destroy_router(router);
            printf("Router %d destroyed\n", i+1);
        } else {
            printf("Failed to create router %d\n", i+1);
        }
    }
    
    // Test 3: Error module
    printf("\n=== ERROR MODULE TEST ===\n");
    for (int i = 0; i < 3; i++) {
        printf("Creating error %d...\n", i+1);
        
        Error *error = create_error(ERROR_NOT_FOUND, "Test error", __FILE__, __LINE__, __func__);
        if (error) {
            printf("Error %d: %d %s\n", i+1, error->status_code, error->message);
            destroy_error(error);
            printf("Error %d destroyed\n", i+1);
        } else {
            printf("Failed to create error %d\n", i+1);
        }
    }
    
    // Test 4: Error context
    printf("\nTesting error context...\n");
    ErrorContext *ctx = create_error_context();
    if (ctx) {
        printf("Error context created\n");
        
        // Set an error
        Error *error = create_error(ERROR_INTERNAL_SERVER_ERROR, "Context test", __FILE__, __LINE__, __func__);
        error_context_set_error(ctx, error);
        printf("Error set in context\n");
        
        destroy_error_context(ctx);
        printf("Error context destroyed\n");
    }
    
    printf("\nAll memory management tests completed!\n");
    return 0;
}
