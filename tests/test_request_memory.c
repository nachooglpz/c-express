#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/http/request.h"

int main() {
    printf("Testing Request memory management...\n");
    
    // Test 1: JSON error allocation leak
    {
        printf("\nTest 1: JSON parsing with wrong content type\n");
        Request req = {0};
        req.client_fd = -1;
        strcpy(req.body, "{\"key\": \"value\"}");
        
        // Set wrong content type to trigger error allocation
        req.header_count = 1;
        strcpy(req.headers[0].key, "Content-Type");
        strcpy(req.headers[0].value, "text/plain");
        
        // This should allocate error message with strdup
        JsonValue *result = request_get_json(&req);
        printf("JSON result: %s\n", result ? "success" : "failed (expected)");
        printf("Error message: %s\n", req.json_error ? req.json_error : "none");
        
        // Cleanup - this should free the strdup'd error message
        request_free_json(&req);
        printf("JSON cleanup completed\n");
    }
    
    // Test 2: Form data error allocation leak
    {
        printf("\nTest 2: Form parsing with wrong content type\n");
        Request req = {0};
        req.client_fd = -1;
        strcpy(req.body, "key=value&key2=value2");
        
        // Set wrong content type to trigger error allocation
        req.header_count = 1;
        strcpy(req.headers[0].key, "Content-Type");
        strcpy(req.headers[0].value, "text/plain");
        
        // This should allocate error message with strdup
        FormData *result = request_get_form(&req);
        printf("Form result: %s\n", result ? "success" : "failed (expected)");
        printf("Error message: %s\n", req.form_data.error_message ? req.form_data.error_message : "none");
        
        // Cleanup - this should free the strdup'd error message
        request_free_form(&req);
        printf("Form cleanup completed\n");
    }
    
    // Test 3: Multiple allocations with proper cleanup (like fixed app.c)
    {
        printf("\nTest 3: Multiple requests with proper cleanup (simulating fixed app.c)\n");
        for (int i = 0; i < 3; i++) {
            Request *req = malloc(sizeof(Request));
            memset(req, 0, sizeof(Request));
            req->client_fd = -1;
            strcpy(req->body, "{\"test\": true}");
            
            // Wrong content type to trigger JSON error
            req->header_count = 1;
            strcpy(req->headers[0].key, "Content-Type");
            strcpy(req->headers[0].value, "text/plain");
            
            JsonValue *json = request_get_json(req);
            FormData *form = request_get_form(req);
            
            printf("Request %d: JSON=%s, Form=%s\n", i+1, 
                   json ? "success" : "failed",
                   form ? "success" : "failed");
            
            // Simulate fixed app.c cleanup (COMPLETE - using request_destroy)
            request_destroy(req);  // This properly frees all allocations!
            free(req);
        }
        printf("Multiple requests test completed (with proper cleanup)\n");
    }
    
    printf("\nRequest memory tests completed!\n");
    return 0;
}
