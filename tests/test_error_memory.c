#include <stdio.h>
#include <stdlib.h>
#include "../src/http/error.h"

int main() {
    printf("Testing Error memory management...\n");
    
    // Test error creation and cleanup
    for (int i = 0; i < 3; i++) {
        printf("Creating error %d...\n", i+1);
        Error *error = create_error(ERROR_NOT_FOUND, "Not Found", __FILE__, __LINE__, __func__);
        
        if (error) {
            printf("Error %d: %d %s\n", i+1, error->status_code, error->message);
            destroy_error(error);
            printf("Error %d destroyed\n", i+1);
        } else {
            printf("Failed to create error %d\n", i+1);
        }
    }
    
    // Test error context
    printf("\nTesting error context...\n");
    ErrorContext *ctx = create_error_context();
    if (ctx) {
        printf("Error context created\n");
        destroy_error_context(ctx);
        printf("Error context destroyed\n");
    }
    
    printf("Error memory tests completed!\n");
    return 0;
}
