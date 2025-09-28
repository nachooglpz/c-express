#include "app.h"
#include <stdio.h>

// API v1 handlers
void api_v1_users(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, "{\"message\":\"API v1 users endpoint\",\"data\":[{\"id\":1,\"name\":\"John\"},{\"id\":2,\"name\":\"Jane\"}]}");
    destroy_response(res);
}

void api_v1_posts(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, "{\"message\":\"API v1 posts endpoint\",\"data\":[{\"id\":1,\"title\":\"Hello World\"},{\"id\":2,\"title\":\"Sub-routers are cool\"}]}");
    destroy_response(res);
}

// API v2 handlers  
void api_v2_users(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, "{\"message\":\"API v2 users endpoint\",\"version\":\"2.0\",\"data\":[{\"id\":1,\"name\":\"John\",\"email\":\"john@example.com\"},{\"id\":2,\"name\":\"Jane\",\"email\":\"jane@example.com\"}]}");
    destroy_response(res);
}

void api_v2_posts(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, "{\"message\":\"API v2 posts endpoint\",\"version\":\"2.0\",\"data\":[{\"id\":1,\"title\":\"Hello World\",\"tags\":[\"hello\",\"world\"]},{\"id\":2,\"title\":\"Sub-routers are amazing\",\"tags\":[\"routers\",\"express\"]}]}");
    destroy_response(res);
}

// Admin handlers
void admin_dashboard(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    response_set_header(res, "Content-Type", "text/html");
    response_send(res, "<h1>Admin Dashboard</h1><p>Welcome to the admin panel</p>");
    destroy_response(res);
}

void admin_stats(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, "{\"message\":\"Admin stats\",\"users\":150,\"posts\":42,\"active_sessions\":7}");
    destroy_response(res);
}

// Main app handler
void home_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next; (void)context;
    Response *res = create_response(client_fd);
    response_set_header(res, "Content-Type", "text/html");
    response_send(res, "<h1>Welcome to C-Express</h1><p>Try these endpoints:</p><ul><li>/api/v1/users</li><li>/api/v1/posts</li><li>/api/v2/users</li><li>/api/v2/posts</li><li>/admin</li><li>/admin/stats</li></ul>");
    destroy_response(res);
}

// Middleware
void api_middleware(int client_fd, void (*next)(void *), void *context) {
    (void)client_fd;
    printf("[MIDDLEWARE] API middleware - request processed\n");
    next(context);
}

void admin_middleware(int client_fd, void (*next)(void *), void *context) {
    (void)client_fd;
    printf("[MIDDLEWARE] Admin middleware - checking permissions\n");
    next(context);
}

int main() {
    printf("=== Testing Sub-Router Functionality ===\n\n");
    
    // Create main app
    App app = create_app();
    
    // Create API v1 router
    Router *api_v1_router = create_router();
    if (!api_v1_router) {
        printf("Failed to create API v1 router\n");
        return 1;
    }
    
    // Add middleware and routes to API v1 router
    api_v1_router->use(api_v1_router, "/", api_middleware);
    api_v1_router->get(api_v1_router, "/users", api_v1_users);
    api_v1_router->get(api_v1_router, "/posts", api_v1_posts);
    
    // Create API v2 router
    Router *api_v2_router = create_router();
    if (!api_v2_router) {
        printf("Failed to create API v2 router\n");
        return 1;
    }
    
    // Add middleware and routes to API v2 router
    api_v2_router->use(api_v2_router, "/", api_middleware);
    api_v2_router->get(api_v2_router, "/users", api_v2_users);
    api_v2_router->get(api_v2_router, "/posts", api_v2_posts);
    
    // Create admin router
    Router *admin_router = create_router();
    if (!admin_router) {
        printf("Failed to create admin router\n");
        return 1;
    }
    
    // Add middleware and routes to admin router
    admin_router->use(admin_router, "/", admin_middleware);
    admin_router->get(admin_router, "/", admin_dashboard);
    admin_router->get(admin_router, "/stats", admin_stats);
    
    // Mount sub-routers on the main app
    app.mount(&app, "/api/v1", api_v1_router);
    app.mount(&app, "/api/v2", api_v2_router);  
    app.mount(&app, "/admin", admin_router);
    
    // Add some main app routes
    app.get(&app, "/", home_handler);
    
    printf("Sub-router test setup complete!\n");
    printf("Router structure:\n");
    printf("  Main App:\n");
    printf("    GET / - Welcome page\n");
    printf("    Mount /api/v1 -> API v1 Router\n");
    printf("      Middleware: API middleware\n");
    printf("      GET /users - API v1 users\n");
    printf("      GET /posts - API v1 posts\n");
    printf("    Mount /api/v2 -> API v2 Router\n");
    printf("      Middleware: API middleware\n");
    printf("      GET /users - API v2 users\n");
    printf("      GET /posts - API v2 posts\n");
    printf("    Mount /admin -> Admin Router\n");
    printf("      Middleware: Admin middleware\n");
    printf("      GET / - Admin dashboard\n");
    printf("      GET /stats - Admin stats\n");
    printf("\nStarting server on port 3000...\n");
    
    // Start server
    app_listen(&app, 3000);
    
    // Cleanup
    destroy_router(api_v1_router);
    destroy_router(api_v2_router);
    destroy_router(admin_router);
    // Note: no destroy_app needed since app is on the stack
    
    return 0;
}
