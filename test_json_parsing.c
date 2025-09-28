#include "app.h"
#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test handler for user creation with JSON validation
void create_user_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next;
    (void)client_fd;  // We'll use the one from context
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    Response *res = (Response *)ctx->user_context;
    
    printf("[DEBUG] create_user_handler: Processing user creation request\n");
    
    // Check content type
    const char *content_type = req->get_header(req, "Content-Type");
    printf("[DEBUG] Content-Type: %s\n", content_type ? content_type : "none");
    
    if (!req->get_json(req)) {
        printf("[DEBUG] Failed to parse JSON or no JSON content\n");
        response_status(res, 400);
        response_set_header(res, "Content-Type", "application/json");
        response_send(res, "{\"error\":\"Invalid or missing JSON\",\"message\":\"Request body must be valid JSON\"}");
        return;
    }
    
    // Create validation schema for user creation
    JsonSchema *user_schema = json_create_schema("create_user");
    json_schema_add_string_field(user_schema, "name", JSON_FIELD_REQUIRED, 2, 50);
    json_schema_add_string_field(user_schema, "email", JSON_FIELD_REQUIRED, 5, 100);
    json_schema_add_number_field(user_schema, "age", JSON_FIELD_OPTIONAL, 0, 150);
    json_schema_add_field(user_schema, "active", JSON_BOOL, JSON_FIELD_OPTIONAL);
    
    // Validate JSON against schema
    if (!req->validate_json_schema(req, user_schema)) {
        printf("[DEBUG] JSON validation failed\n");
        response_status(res, 400);
        response_set_header(res, "Content-Type", "application/json");
        response_send(res, "{\"error\":\"Validation failed\",\"message\":\"Request does not match required schema\"}");
        json_free_schema(user_schema);
        return;
    }
    
    json_free_schema(user_schema);
    
    // Extract values from JSON
    const char *name = req->get_json_string(req, "name");
    const char *email = req->get_json_string(req, "email");
    double age = req->get_json_number(req, "age");
    int active = req->get_json_bool(req, "active");
    
    printf("[DEBUG] Parsed JSON values - name: %s, email: %s, age: %.0f, active: %d\n",
           name ? name : "null", email ? email : "null", age, active);
    
    // Create response
    char response_body[1024];
    snprintf(response_body, sizeof(response_body),
        "{"
        "\"id\": 12345,"
        "\"message\": \"User created successfully\","
        "\"user\": {"
        "  \"name\": \"%s\","
        "  \"email\": \"%s\","
        "  \"age\": %.0f,"
        "  \"active\": %s"
        "}"
        "}",
        name ? name : "",
        email ? email : "",
        age,
        active ? "true" : "false");
    
    response_status(res, 201);
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response_body);
}

// Test handler for updating user profile with nested JSON
void update_profile_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next;
    (void)client_fd;
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    Response *res = (Response *)ctx->user_context;
    
    const char *user_id = req->get_param(req, "id");
    printf("[DEBUG] update_profile_handler: Updating profile for user %s\n", user_id ? user_id : "unknown");
    
    JsonValue *json = req->get_json(req);
    if (!json) {
        response_status(res, 400);
        response_set_header(res, "Content-Type", "application/json");
        response_send(res, "{\"error\":\"Invalid JSON\"}");
        return;
    }
    
    // Handle nested objects
    JsonObject *profile = req->get_json_object(req, "profile");
    JsonArray *interests = req->get_json_array(req, "interests");
    
    char response_body[1024];
    snprintf(response_body, sizeof(response_body),
        "{"
        "\"message\": \"Profile updated for user %s\","
        "\"has_profile\": %s,"
        "\"has_interests\": %s,"
        "\"interests_count\": %d"
        "}",
        user_id ? user_id : "unknown",
        profile ? "true" : "false",
        interests ? "true" : "false",
        interests ? json_array_size(interests) : 0);
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response_body);
}

// Test handler for bulk operations with JSON arrays
void bulk_operation_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next;
    NextContext *ctx = (NextContext *)context;
    Response *res = create_response(client_fd);
    Request *req = ctx->req;
    
    printf("[DEBUG] bulk_operation_handler: Processing bulk operation\n");
    
    JsonValue *json = req->get_json(req);
    if (!json || json->type != JSON_ARRAY) {
        response_status(res, 400);
        response_set_header(res, "Content-Type", "application/json");
        response_send(res, "{\"error\":\"Expected JSON array\"}");
        return;
    }
    
    JsonArray *items = json->data.array_value;
    int item_count = json_array_size(items);
    
    printf("[DEBUG] Processing %d items in bulk operation\n", item_count);
    
    char response_body[512];
    snprintf(response_body, sizeof(response_body),
        "{"
        "\"message\": \"Bulk operation completed\","
        "\"processed_count\": %d,"
        "\"status\": \"success\""
        "}",
        item_count);
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response_body);
}

// Test handler to echo parsed JSON back
void json_echo_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next;
    Response *res = create_response(client_fd);
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    
    printf("[DEBUG] json_echo_handler: Echoing JSON back to client\n");
    
    JsonValue *json = req->get_json(req);
    if (!json) {
        response_status(res, 400);
        response_set_header(res, "Content-Type", "application/json");
        response_send(res, "{\"error\":\"No valid JSON found\"}");
        return;
    }
    
    printf("[DEBUG] JSON type: %s\n", json_type_name(json->type));
    
    // Echo back the received JSON with metadata
    char response_body[2048];
    snprintf(response_body, sizeof(response_body),
        "{"
        "\"message\": \"JSON received and parsed successfully\","
        "\"original_body\": \"%.500s\","
        "\"json_type\": \"%s\","
        "\"timestamp\": \"2025-09-28T10:00:00Z\""
        "}",
        req->body,
        json_type_name(json->type));
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response_body);
}

// Test non-JSON endpoint for comparison
void plain_text_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    
    response_set_header(res, "Content-Type", "text/plain");
    response_send(res, "This endpoint expects plain text, not JSON");
}

int main() {
    printf("=== Testing JSON Request Body Parsing ===\n\n");
    
    App app = create_app();
    
    printf("Setting up JSON API endpoints...\n\n");
    
    // JSON endpoints
    printf("  POST /users - Create user with JSON validation\n");
    app.post(&app, "/users", create_user_handler);
    
    printf("  PUT /users/:id:number - Update user profile with nested JSON\n");
    app.put(&app, "/users/:id:number", update_profile_handler);
    
    printf("  POST /bulk - Bulk operations with JSON array\n");
    app.post(&app, "/bulk", bulk_operation_handler);
    
    printf("  POST /echo - Echo JSON back with metadata\n");
    app.post(&app, "/echo", json_echo_handler);
    
    printf("  GET /plain - Non-JSON endpoint for comparison\n");
    app.get(&app, "/plain", plain_text_handler);
    
    printf("\n=== JSON Parsing Features ===\n");
    printf("Automatic JSON parsing from request body\n");
    printf("Content-Type validation (application/json)\n");
    printf("JSON schema validation with field types\n");
    printf("Easy access to JSON values by key\n");
    printf("Support for nested objects and arrays\n");
    printf("Comprehensive error handling\n");
    printf("Memory-safe JSON parsing and cleanup\n");
    
    printf("\n=== Testing Instructions ===\n");
    printf("Starting server on port 3000...\n");
    printf("\nTest these JSON endpoints:\n\n");
    
    printf("1. Create User (with validation):\n");
    printf("   curl -X POST http://localhost:3000/users \\\n");
    printf("     -H 'Content-Type: application/json' \\\n");
    printf("     -d '{\"name\":\"Alice\",\"email\":\"alice@test.com\",\"age\":25,\"active\":true}'\n\n");
    
    printf("2. Invalid JSON (should fail validation):\n");
    printf("   curl -X POST http://localhost:3000/users \\\n");
    printf("     -H 'Content-Type: application/json' \\\n");
    printf("     -d '{\"name\":\"X\",\"age\":\"invalid\"}'\n\n");
    
    printf("3. Update Profile (nested JSON):\n");
    printf("   curl -X PUT http://localhost:3000/users/123 \\\n");
    printf("     -H 'Content-Type: application/json' \\\n");
    printf("     -d '{\"profile\":{\"bio\":\"Developer\"},\"interests\":[\"coding\",\"music\"]}'\n\n");
    
    printf("4. Bulk Operation (JSON array):\n");
    printf("   curl -X POST http://localhost:3000/bulk \\\n");
    printf("     -H 'Content-Type: application/json' \\\n");
    printf("     -d '[{\"id\":1,\"action\":\"create\"},{\"id\":2,\"action\":\"update\"}]'\n\n");
    
    printf("5. JSON Echo:\n");
    printf("   curl -X POST http://localhost:3000/echo \\\n");
    printf("     -H 'Content-Type: application/json' \\\n");
    printf("     -d '{\"test\":\"hello\",\"number\":42,\"array\":[1,2,3]}'\n\n");
    
    printf("6. Non-JSON endpoint:\n");
    printf("   curl http://localhost:3000/plain\n\n");
    
    printf("7. Missing Content-Type (should fail):\n");
    printf("   curl -X POST http://localhost:3000/users -d '{\"name\":\"test\"}'\n\n");
    
    app_listen(&app, 3000);
    
    return 0;
}
