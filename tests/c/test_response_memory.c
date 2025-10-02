#include <stdio.h>
#include <stdlib.h>
#include "../src/http/response.h"

int main() {
    printf("Testing Response memory management...\n");
    
    // Test create/destroy pattern
    for (int i = 0; i < 5; i++) {
        printf("Creating response %d...\n", i+1);
        Response *res = create_response(-1);  // Use invalid fd for testing
        
        if (res) {
            printf("Response %d created successfully\n", i+1);
            destroy_response(res);
            printf("Response %d destroyed\n", i+1);
        } else {
            printf("Failed to create response %d\n", i+1);
        }
    }
    
    printf("Response memory tests completed!\n");
    return 0;
}
