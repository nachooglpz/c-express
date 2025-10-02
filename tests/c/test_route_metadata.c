#include "../src/core/app.h"
#include "../src/core/route.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Handler functions for different API endpoints
void users_list_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next;
    Response *res = create_response(client_fd);
    
    char response[] = "{"
        "\"users\": ["
        "  {\"id\": 1, \"name\": \"Alice\", \"email\": \"alice@example.com\"},"
        "  {\"id\": 2, \"name\": \"Bob\", \"email\": \"bob@example.com\"}"
        "],"
        "\"total\": 2,"
        "\"page\": 1"
        "}";
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response);
    destroy_response(res);
}

void user_get_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next;
    Response *res = create_response(client_fd);
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    
    const char *user_id = req->get_param(req, "id");
    
    char response[512];
    snprintf(response, sizeof(response),
        "{"
        "\"id\": %s,"
        "\"name\": \"User %s\","
        "\"email\": \"user%s@example.com\","
        "\"created_at\": \"2025-09-28T10:00:00Z\""
        "}",
        user_id ? user_id : "0",
        user_id ? user_id : "0", 
        user_id ? user_id : "0");
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response);
    destroy_response(res);
}

void user_create_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next;
    Response *res = create_response(client_fd);
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    
    char response[512];
    snprintf(response, sizeof(response),
        "{"
        "\"id\": 123,"
        "\"message\": \"User created successfully\","
        "\"received_data\": \"%.100s\""
        "}",
        req->body ? req->body : "{}");
    
    response_status(res, 201);
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response);
    destroy_response(res);
}

void user_update_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next;
    Response *res = create_response(client_fd);
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    
    const char *user_id = req->get_param(req, "id");
    
    char response[512];
    snprintf(response, sizeof(response),
        "{"
        "\"id\": %s,"
        "\"message\": \"User updated successfully\","
        "\"updated_data\": \"%.100s\""
        "}",
        user_id ? user_id : "0",
        req->body ? req->body : "{}");
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response);
    destroy_response(res);
}

void user_delete_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next;
    Response *res = create_response(client_fd);
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    
    const char *user_id = req->get_param(req, "id");
    
    char response[256];
    snprintf(response, sizeof(response),
        "{"
        "\"message\": \"User %s deleted successfully\""
        "}",
        user_id ? user_id : "0");
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response);
    destroy_response(res);
}

void health_check_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    
    char response[] = "{"
        "\"status\": \"healthy\","
        "\"timestamp\": \"2025-09-28T10:00:00Z\","
        "\"version\": \"1.0.0\","
        "\"uptime\": \"5m 23s\""
        "}";
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response);
    destroy_response(res);
}

void deprecated_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    
    char response[] = "{"
        "\"message\": \"This endpoint is deprecated\","
        "\"warning\": \"Please use /api/v2/legacy-endpoint instead\","
        "\"deprecation_date\": \"2025-10-01\""
        "}";
    
    response_set_header(res, "Content-Type", "application/json");
    response_set_header(res, "X-Deprecated", "true");
    response_send(res, response);
    destroy_response(res);
}

// Test route metadata and documentation system
int main() {
    printf("=== Testing Route Metadata & Documentation ===\n\n");
    
    App app = create_app();
    
    printf("Setting up API routes with comprehensive metadata...\n\n");
    
    // User management API with metadata
    const char *user_tags[] = {"users", "management", NULL};
    const char *system_tags[] = {"system", "monitoring", NULL};
    const char *deprecated_tags[] = {"deprecated", "legacy", NULL};
    
    RouteConfig list_users_config = {
        .route_id = "list_users",
        .summary = "List all users",
        .description = "Retrieve a paginated list of all users in the system",
        .tags = user_tags,
        .deprecated = 0,
        .deprecated_reason = NULL
    };
    
    RouteConfig get_user_config = {
        .route_id = "get_user",
        .summary = "Get user by ID", 
        .description = "Retrieve detailed information about a specific user",
        .tags = user_tags,
        .deprecated = 0,
        .deprecated_reason = NULL
    };
    
    RouteConfig create_user_config = {
        .route_id = "create_user",
        .summary = "Create new user",
        .description = "Create a new user account with provided information",
        .tags = user_tags,
        .deprecated = 0,
        .deprecated_reason = NULL
    };
    
    RouteConfig update_user_config = {
        .route_id = "update_user", 
        .summary = "Update user",
        .description = "Update existing user information",
        .tags = user_tags,
        .deprecated = 0,
        .deprecated_reason = NULL
    };
    
    RouteConfig delete_user_config = {
        .route_id = "delete_user",
        .summary = "Delete user",
        .description = "Permanently delete a user account",
        .tags = user_tags,
        .deprecated = 0,
        .deprecated_reason = NULL
    };
    
    RouteConfig health_config = {
        .route_id = "health_check",
        .summary = "Health check",
        .description = "Check system health and status",
        .tags = system_tags,
        .deprecated = 0,
        .deprecated_reason = NULL
    };
    
    RouteConfig deprecated_config = {
        .route_id = "legacy_endpoint",
        .summary = "Legacy endpoint",
        .description = "Old API endpoint maintained for backward compatibility",
        .tags = deprecated_tags,
        .deprecated = 1,
        .deprecated_reason = "Use /api/v2/legacy-endpoint instead. Will be removed in v2.0"
    };
    
    printf("Registering routes with enhanced metadata:\n");
    
    // Register routes with metadata
    printf("  GET /users - List users\n");
    app_get_with_metadata(&app, "/users", users_list_handler, &list_users_config);
    
    printf("  GET /users/:id:number - Get user by ID\n");
    app_get_with_metadata(&app, "/users/:id:number", user_get_handler, &get_user_config);
    
    printf("  POST /users - Create user\n");
    app_post_with_metadata(&app, "/users", user_create_handler, &create_user_config);
    
    printf("  PUT /users/:id:number - Update user\n");
    app_put_with_metadata(&app, "/users/:id:number", user_update_handler, &update_user_config);
    
    printf("  DELETE /users/:id:number - Delete user\n");
    app_delete_with_metadata(&app, "/users/:id:number", user_delete_handler, &delete_user_config);
    
    printf("  GET /health - Health check\n");
    app_get_with_metadata(&app, "/health", health_check_handler, &health_config);
    
    printf("  GET /api/v1/legacy - Deprecated endpoint\n");
    app_get_with_metadata(&app, "/api/v1/legacy", deprecated_handler, &deprecated_config);
    
    printf("\n=== Route Metadata Features ===\n");
    printf("Route IDs for programmatic access\n");
    printf("Summaries and descriptions\n");
    printf("Tagging system for organization\n");
    printf("Deprecation warnings\n");
    printf("Enhanced route registration\n");
    
    printf("\n=== Testing Documentation Generation ===\n");
    
    // Test standalone route metadata
    printf("\nCreating standalone route metadata example:\n");
    RouteMetadata *demo_meta = create_route_metadata();
    set_route_summary(demo_meta, "Demo endpoint");
    set_route_description(demo_meta, "This is a demonstration of route metadata capabilities");
    add_route_tag(demo_meta, "demo");
    add_route_tag(demo_meta, "testing");
    
    // Add parameter documentation
    add_parameter_doc(demo_meta, "id", PARAM_NUMBER, 1, "User identifier", "123");
    add_parameter_doc(demo_meta, "format", PARAM_STRING, 0, "Response format", "json");
    
    // Add response documentation
    add_response_doc(demo_meta, 200, "Success response", "application/json", "{\"status\":\"ok\"}");
    add_response_doc(demo_meta, 404, "User not found", "application/json", "{\"error\":\"Not found\"}");
    
    // Add examples
    add_route_example(demo_meta, "GET /demo/123?format=json");
    add_route_example(demo_meta, "GET /demo/456?format=xml");
    
    printf("Demo metadata created with:\n");
    printf("  - Summary and description\n");
    printf("  - 2 tags\n");
    printf("  - 2 parameter docs\n");
    printf("  - 2 response docs\n");
    printf("  - 2 examples\n");
    
    // Test OpenAPI generation for demo route
    RoutePattern *demo_pattern = compile_route_pattern("/demo/:id:number");
    Route demo_route = {
        .pattern = demo_pattern,
        .metadata = demo_meta,
        .route_id = strdup("demo_route")
    };
    
    printf("\nGenerating OpenAPI JSON for demo route:\n");
    char *openapi_json = generate_route_openapi_json(&demo_route, "/api/v1");
    if (openapi_json) {
        printf("Generated OpenAPI JSON:\n%s\n", openapi_json);
        free(openapi_json);
    }
    
    // Cleanup demo resources
    free_route_metadata(demo_meta);
    free_route_pattern(demo_pattern);
    free(demo_route.route_id);
    
    printf("\n=== Server Starting ===\n");
    printf("Starting server on port 3000...\n");
    printf("\nTest these documented endpoints:\n");
    printf("  User Management:\n");
    printf("    curl http://localhost:3000/users\n");
    printf("    curl http://localhost:3000/users/123\n");
    printf("    curl -X POST -H 'Content-Type: application/json' -d '{\"name\":\"John\"}' http://localhost:3000/users\n");
    printf("    curl -X PUT -H 'Content-Type: application/json' -d '{\"name\":\"Jane\"}' http://localhost:3000/users/123\n");
    printf("    curl -X DELETE http://localhost:3000/users/123\n");
    printf("  System:\n");
    printf("    curl http://localhost:3000/health\n");
    printf("  Deprecated (with warnings):\n");
    printf("    curl http://localhost:3000/api/v1/legacy\n");
    printf("\n");
    
    // Print route information
    app_print_routes(&app);
    
    app_listen(&app, 3000);
    
    return 0;
}
