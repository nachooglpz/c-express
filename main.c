#include "app.h"
#include "response.h"
#include "request.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// first middleware for GET /
void log_handler(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    printf("Log middleware: %s %s\n", req->method, req->path);
    
    // Log query parameters if any
    if (req->query_count > 0) {
        printf("Query parameters:\n");
        for (int i = 0; i < req->query_count; i++) {
            printf("  %s = %s\n", req->query[i].key, req->query[i].value);
        }
    }
    
    next(ctx);
}

// second middleware for GET /
void hello_handler(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = (Response *)ctx->user_context;
    Request *req = ctx->req;
    
    printf("[DEBUG] hello_handler: sending the response\n");
    
    // Check for name query parameter
    const char *name = req->get_query(req, "name");
    if (name) {
        char message[256];
        snprintf(message, sizeof(message), "Hello %s!", name);
        res->send(res, message);
    } else {
        res->send(res, "Hello World!");
    }
}

// handler for GET /users/:id
void user_handler(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = (Response *)ctx->user_context;
    Request *req = ctx->req;
    
    const char *user_id = req->get_param(req, "id");
    if (user_id) {
        char message[256];
        snprintf(message, sizeof(message), "User ID: %s", user_id);
        res->send(res, message);
    } else {
        res->send(res, "No user ID provided");
    }
}

// handler for GET /2
void hello_handler2(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = (Response *)ctx->user_context;
    res->send(res, "Hello World 2!");
}

// handler to send a json
void json_handler(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = (Response *)ctx->user_context;
    Request *req = ctx->req;
    
    // Check for format query parameter
    const char *format = req->get_query(req, "format");
    if (format && strcmp(format, "detailed") == 0) {
        res->json(res, "{\"hello\":\"world\",\"message\":\"detailed response\"}");
    } else {
        res->json(res, "{\"hello\":\"world\"}");
    }
}

// handler for POST /post
void post_handler(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = (Response *)ctx->user_context;
    Request *req = ctx->req;
    
    printf("[DEBUG] post_handler: sending response\n");
    printf("[DEBUG] post_handler: request body: %s\n", req->body);
    
    // Set 201 Created status for POST requests
    res->status(res, 201);
    
    // Echo the request body
    char response_body[512];
    snprintf(response_body, sizeof(response_body), 
             "POST received! Body: %s", req->body);
    res->send(res, response_body);
}

// handler for PUT /users/:id
void put_handler(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = (Response *)ctx->user_context;
    Request *req = ctx->req;
    
    const char *user_id = req->get_param(req, "id");
    printf("[DEBUG] put_handler: updating user %s\n", user_id ? user_id : "unknown");
    printf("[DEBUG] put_handler: request body: %s\n", req->body);
    
    char response_body[512];
    snprintf(response_body, sizeof(response_body), 
             "PUT received! Updated user %s with data: %s", 
             user_id ? user_id : "unknown", req->body);
    res->send(res, response_body);
}

// handler for DELETE /users/:id
void delete_handler(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = (Response *)ctx->user_context;
    Request *req = ctx->req;
    
    const char *user_id = req->get_param(req, "id");
    printf("[DEBUG] delete_handler: deleting user %s\n", user_id ? user_id : "unknown");
    
    char response_body[256];
    snprintf(response_body, sizeof(response_body), 
             "DELETE received! Deleted user %s", 
             user_id ? user_id : "unknown");
    res->send(res, response_body);
}

// handler for PATCH /users/:id
void patch_handler(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = (Response *)ctx->user_context;
    Request *req = ctx->req;
    
    const char *user_id = req->get_param(req, "id");
    printf("[DEBUG] patch_handler: patching user %s\n", user_id ? user_id : "unknown");
    printf("[DEBUG] patch_handler: request body: %s\n", req->body);
    
    char response_body[512];
    snprintf(response_body, sizeof(response_body), 
             "PATCH received! Patched user %s with data: %s", 
             user_id ? user_id : "unknown", req->body);
    res->send(res, response_body);
}

// handler for OPTIONS /api
void options_handler(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = (Response *)ctx->user_context;
    
    printf("[DEBUG] options_handler: CORS preflight request\n");
    
    // Set CORS headers
    res->set_header(res, "Access-Control-Allow-Origin", "*");
    res->set_header(res, "Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS");
    res->set_header(res, "Access-Control-Allow-Headers", "Content-Type, Authorization");
    
    res->send(res, "");
}

// handler for GET /status/:code - demonstrates different status codes
void status_test_handler(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = (Response *)ctx->user_context;
    Request *req = ctx->req;
    
    const char *status_code_str = req->get_param(req, "code");
    
    if (status_code_str) {
        int status_code = atoi(status_code_str);
        printf("[DEBUG] status_test_handler: setting status %d\n", status_code);
        
        // Use send_status to send just the status text
        res->send_status(res, status_code);
    } else {
        res->status(res, 400);
        res->send(res, "Bad Request: status code parameter required");
    }
}

int main() {
    App app = create_app();

    // Add logging middleware
    app.use(&app, log_handler);
    
    // GET routes
    app.get(&app, "/", hello_handler);
    app.get(&app, "/2", hello_handler2);
    app.get(&app, "/users/:id", user_handler);
    app.get(&app, "/json", json_handler);
    app.get(&app, "/status/:code", status_test_handler);  // Test different status codes
    
    // POST routes
    app.post(&app, "/post", post_handler);
    
    // PUT routes
    app.put(&app, "/users/:id", put_handler);
    
    // DELETE routes
    app.delete(&app, "/users/:id", delete_handler);
    
    // PATCH routes
    app.patch(&app, "/users/:id", patch_handler);
    
    // OPTIONS routes (for CORS)
    app.options(&app, "/api", options_handler);
    
    app.listen(&app, 3000);

    free(app.router.layers);
    return 0;
}