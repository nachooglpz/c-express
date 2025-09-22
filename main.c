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
    
    // Echo the request body
    char response_body[512];
    snprintf(response_body, sizeof(response_body), 
             "POST received! Body: %s", req->body);
    res->send(res, response_body);
}

int main() {
    App app = create_app();

    // Add logging middleware
    app.use(&app, log_handler);
    
    app.get(&app, "/", hello_handler);
    app.get(&app, "/2", hello_handler2);
    app.get(&app, "/users/:id", user_handler);
    app.post(&app, "/post", post_handler);
    app.get(&app, "/json", json_handler);
    app.listen(&app, 3000);

    free(app.router.layers);
    return 0;
}