#include "../src/core/app.h"
#include <stdio.h>

// Handler for user by ID
void get_user_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, "{\"message\":\"User endpoint with ID parameter\"}");
    destroy_response(res);
}

// Handler for posts with slug
void get_post_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, "{\"message\":\"Post endpoint\",\"note\":\"Slug parameter should be extracted\"}");
    destroy_response(res);
}

// Handler for file downloads with wildcard
void download_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, "{\"message\":\"File download\",\"note\":\"Wildcard path should be captured\"}");
    destroy_response(res);
}

// Handler for optional parameters
void user_posts_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, "{\"message\":\"User posts\",\"note\":\"Post ID is optional\"}");
    destroy_response(res);
}

// Handler for number validation
void get_number_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, "{\"message\":\"Number endpoint\",\"note\":\"Only numbers should match\"}");
    destroy_response(res);
}

// Handler for UUID validation
void get_uuid_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, "{\"message\":\"UUID endpoint\",\"note\":\"Only valid UUIDs should match\"}");
    destroy_response(res);
}

int main() {
    printf("=== Testing Advanced Pattern Matching ===\n\n");
    
    // Create main app
    App app = create_app();
    
    // Test routes with different parameter types
    printf("Setting up routes with advanced patterns:\n");
    
    // 1. Simple parameter
    app.get(&app, "/users/:id", get_user_handler);
    printf("  GET /users/:id - Simple parameter\n");
    
    // 2. Named parameter with type
    app.get(&app, "/numbers/:num:number", get_number_handler);
    printf("  GET /numbers/:num:number - Number parameter\n");
    
    // 3. UUID parameter
    app.get(&app, "/resources/:uuid:uuid", get_uuid_handler);
    printf("  GET /resources/:uuid:uuid - UUID parameter\n");
    
    // 4. Slug parameter
    app.get(&app, "/posts/:slug:slug", get_post_handler);
    printf("  GET /posts/:slug:slug - Slug parameter\n");
    
    // 5. Optional parameters
    app.get(&app, "/users/:id/posts/:postId?", user_posts_handler);
    printf("  GET /users/:id/posts/:postId? - Optional post ID\n");
    
    // 6. Wildcard routes
    app.get(&app, "/files/*", download_handler);
    printf("  GET /files/* - Wildcard route\n");
    
    printf("\nStarting server on port 3000...\n");
    printf("\nTest these endpoints:\n");
    printf("  curl http://localhost:3000/users/123\n");
    printf("  curl http://localhost:3000/users/abc (should work)\n");
    printf("  curl http://localhost:3000/numbers/456\n");
    printf("  curl http://localhost:3000/numbers/abc (should 404)\n");
    printf("  curl http://localhost:3000/resources/550e8400-e29b-41d4-a716-446655440000\n");
    printf("  curl http://localhost:3000/resources/invalid-uuid (should 404)\n");
    printf("  curl http://localhost:3000/posts/my-awesome-post\n");
    printf("  curl http://localhost:3000/posts/invalid slug! (should 404)\n");
    printf("  curl http://localhost:3000/users/123/posts\n");
    printf("  curl http://localhost:3000/users/123/posts/456\n");
    printf("  curl http://localhost:3000/files/docs/readme.txt\n");
    printf("  curl http://localhost:3000/files/images/photo.jpg\n\n");
    
    // Start server
    app_listen(&app, 3000);
    
    return 0;
}
