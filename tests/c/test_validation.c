#include "../src/core/app.h"
#include "../src/core/route.h"
#include <stdio.h>

// Custom validator example - check if username doesn't contain profanity
int username_validator(const char *value, void *context, ValidationError *error) {
    (void)context;
    
    // Simple profanity check
    const char *banned_words[] = {"admin", "root", "test", NULL};
    
    for (int i = 0; banned_words[i]; i++) {
        if (strstr(value, banned_words[i])) {
            if (error) {
                error->error_message = strdup("Username contains reserved word");
            }
            return 0;
        }
    }
    
    return 1;
}

// Handler functions
void user_by_id_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    
    // Get the request to access parameters
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    
    const char *id = req->get_param(req, "id");
    
    char response_body[512];
    snprintf(response_body, sizeof(response_body),
             "{\"message\":\"User endpoint\",\"id\":\"%s\",\"note\":\"ID must be 1-999\"}", 
             id ? id : "unknown");
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response_body);
    destroy_response(res);
}

void age_restricted_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    
    const char *age = req->get_param(req, "age");
    
    char response_body[512];
    snprintf(response_body, sizeof(response_body),
             "{\"message\":\"Age restricted content\",\"age\":\"%s\",\"note\":\"Age must be 18-120\"}", 
             age ? age : "unknown");
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response_body);
    destroy_response(res);
}

void category_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    
    const char *category = req->get_param(req, "category");
    
    char response_body[512];
    snprintf(response_body, sizeof(response_body),
             "{\"message\":\"Category endpoint\",\"category\":\"%s\",\"note\":\"Must be electronics, books, or clothing\"}", 
             category ? category : "unknown");
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response_body);
    destroy_response(res);
}

void username_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    
    const char *username = req->get_param(req, "username");
    
    char response_body[512];
    snprintf(response_body, sizeof(response_body),
             "{\"message\":\"Username profile\",\"username\":\"%s\",\"note\":\"Username validation passed\"}", 
             username ? username : "unknown");
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response_body);
    destroy_response(res);
}

void phone_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    
    const char *phone = req->get_param(req, "phone");
    
    char response_body[512];
    snprintf(response_body, sizeof(response_body),
             "{\"message\":\"Phone verification\",\"phone\":\"%s\",\"note\":\"Phone number format validated\"}", 
             phone ? phone : "unknown");
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response_body);
    destroy_response(res);
}

void validation_error_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    
    response_set_header(res, "Content-Type", "application/json");
    response_status(res, 400);
    response_send(res, "{\"error\":\"Validation failed\",\"message\":\"Parameter validation constraints were not met\"}");
    destroy_response(res);
}

int main() {
    printf("=== Testing Route Validation and Constraints ===\n\n");
    
    // Create main app
    App app = create_app();
    
    printf("Setting up routes with validation constraints:\n");
    
    // Route 1: User ID with range constraint (1-999)
    app.get(&app, "/users/:id:number", user_by_id_handler);
    printf("  GET /users/:id:number - ID must be 1-999\n");
    
    // Route 2: Age restriction (18-120)
    app.get(&app, "/age/:age:number", age_restricted_handler);
    printf("  GET /age/:age:number - Age must be 18-120\n");
    
    // Route 3: Category enum constraint
    app.get(&app, "/category/:category", category_handler);
    printf("  GET /category/:category - Must be electronics, books, or clothing\n");
    
    // Route 4: Username with custom validation
    app.get(&app, "/user/:username", username_handler);
    printf("  GET /user/:username - Custom validation (no reserved words)\n");
    
    // Route 5: Phone number with regex pattern
    app.get(&app, "/phone/:phone", phone_handler);
    printf("  GET /phone/:phone - Must match phone number pattern\n");
    
    printf("\nStarting server on port 3000...\n");
    printf("\nTest these endpoints:\n");
    printf("  Valid cases:\n");
    printf("    curl http://localhost:3000/users/123 (valid: 1-999)\n");
    printf("    curl http://localhost:3000/users/1 (valid: minimum)\n");
    printf("    curl http://localhost:3000/users/999 (valid: maximum)\n");
    printf("    curl http://localhost:3000/age/25 (valid: 18-120)\n");
    printf("    curl http://localhost:3000/category/electronics (valid enum)\n");
    printf("    curl http://localhost:3000/user/johndoe (valid username)\n");
    printf("    curl http://localhost:3000/phone/555-123-4567 (valid phone)\n");
    printf("\n  Invalid cases (should return 400):\n");
    printf("    curl http://localhost:3000/users/0 (too small)\n");
    printf("    curl http://localhost:3000/users/1000 (too large)\n");
    printf("    curl http://localhost:3000/age/15 (under 18)\n");
    printf("    curl http://localhost:3000/age/150 (over 120)\n");
    printf("    curl http://localhost:3000/category/invalid (not in enum)\n");
    printf("    curl http://localhost:3000/user/admin (reserved word)\n");
    printf("    curl http://localhost:3000/phone/invalid (bad format)\n");
    printf("\n");
    
    // Start server
    app_listen(&app, 3000);
    
    return 0;
}
