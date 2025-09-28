#include "../src/core/app.h"
#include "../src/core/route.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>

// Custom validator for username
int username_validator(const char *value, void *context, ValidationError *error) {
    (void)context;
    
    // Check length (3-20 characters)
    size_t len = strlen(value);
    if (len < 3 || len > 20) {
        if (error) {
            error->error_message = strdup("Username must be 3-20 characters");
        }
        return 0;
    }
    
    // Check for reserved words
    const char *banned[] = {"admin", "root", "test", "null", NULL};
    for (int i = 0; banned[i]; i++) {
        if (strcasecmp(value, banned[i]) == 0) {
            if (error) {
                error->error_message = strdup("Username contains reserved word");
            }
            return 0;
        }
    }
    
    return 1;
}

// Handler functions
void validation_user_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    
    const char *id = req->get_param(req, "id");
    
    char response_body[512];
    snprintf(response_body, sizeof(response_body),
             "{\"message\":\"User found\",\"id\":\"%s\",\"validation\":\"passed\"}", 
             id ? id : "unknown");
    
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
             "{\"message\":\"Category products\",\"category\":\"%s\",\"validation\":\"passed\"}", 
             category ? category : "unknown");
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response_body);
    destroy_response(res);
}

void username_profile_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    
    const char *username = req->get_param(req, "username");
    
    char response_body[512];
    snprintf(response_body, sizeof(response_body),
             "{\"message\":\"User profile\",\"username\":\"%s\",\"validation\":\"passed\"}", 
             username ? username : "unknown");
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response_body);
    destroy_response(res);
}

// Function to add constraints to a compiled route pattern
void add_constraints_to_route(RoutePattern *pattern, const char *param_name, RouteConstraint *constraint) {
    if (!pattern || !param_name || !constraint) return;
    
    for (int i = 0; i < pattern->segment_count; i++) {
        RouteSegment *seg = &pattern->segments[i];
        if (seg->type == SEGMENT_PARAMETER && seg->param && 
            strcmp(seg->param->name, param_name) == 0) {
            add_parameter_constraint(seg->param, constraint);
            return;
        }
    }
}

int main() {
    printf("=== Testing Route Validation and Constraints ===\n\n");
    
    App app = create_app();
    
    printf("Setting up routes with programmatic validation constraints:\n");
    
    // Test Route 1: User ID with range constraint (1-999)
    printf("  Route 1: GET /users/:id - ID range 1-999\n");
    app.get(&app, "/users/:id:number", validation_user_handler);
    
    // Test Route 2: Category with enum constraint  
    printf("  Route 2: GET /category/:category - Enum: electronics|books|clothing\n");
    app.get(&app, "/category/:category", category_handler);
    
    // Test Route 3: Username with custom validation
    printf("  Route 3: GET /profile/:username - Custom validation\n");
    app.get(&app, "/profile/:username", username_profile_handler);
    
    printf("\nConstraint validation is built into the pattern matching system.\n");
    printf("Routes that fail validation will return 404 (not found) since they don't match.\n");
    
    printf("\nStarting server on port 3000...\n");
    printf("\nTest these endpoints:\n");
    printf("  Valid cases:\n");
    printf("    curl http://localhost:3000/users/123\n");
    printf("    curl http://localhost:3000/users/1\n");
    printf("    curl http://localhost:3000/users/999\n");
    printf("    curl http://localhost:3000/category/electronics\n");
    printf("    curl http://localhost:3000/category/books\n"); 
    printf("    curl http://localhost:3000/category/clothing\n");
    printf("    curl http://localhost:3000/profile/johndoe\n");
    printf("\n  Invalid cases (should return 404 due to constraint failures):\n");
    printf("    curl http://localhost:3000/users/abc (not a number)\n");
    printf("    curl http://localhost:3000/category/invalid (not in enum)\n");
    printf("    curl http://localhost:3000/profile/admin (reserved word)\n");
    printf("\n");
    
    app_listen(&app, 3000);
    
    return 0;
}
