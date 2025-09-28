#include "src/core/app.h"
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

// Error-prone handler that demonstrates error throwing
void error_test_handler(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = (Response *)ctx->user_context;
    Request *req = ctx->req;
    
    const char *error_type = req->get_param(req, "type");
    
    TRY(ctx->error_ctx)
        if (!error_type) {
            THROW_ERROR(ctx->error_ctx, ERROR_BAD_REQUEST, "Missing error type parameter");
            return;
        }
        
        if (strcmp(error_type, "400") == 0) {
            THROW_ERROR(ctx->error_ctx, ERROR_BAD_REQUEST, "This is a test bad request error");
        } else if (strcmp(error_type, "401") == 0) {
            THROW_ERROR(ctx->error_ctx, ERROR_UNAUTHORIZED, "You are not authorized to access this resource");
        } else if (strcmp(error_type, "403") == 0) {
            THROW_ERROR(ctx->error_ctx, ERROR_FORBIDDEN, "Access to this resource is forbidden");
        } else if (strcmp(error_type, "500") == 0) {
            THROW_ERROR(ctx->error_ctx, ERROR_INTERNAL_SERVER_ERROR, "Something went wrong on the server");
        } else {
            // This should work fine
            res->send(res, "No error thrown - everything is working fine!");
        }
    CATCH(ctx->error_ctx)
        printf("[DEBUG] error_test_handler: Caught error in handler\n");
        // Error will be handled by the error middleware
        return;
    FINALLY(ctx->error_ctx)
        printf("[DEBUG] error_test_handler: Finally block executed\n");
}

// Custom error handler
void custom_error_handler(Error *error, int client_fd, void *context) {
    printf("[DEBUG] custom_error_handler: Handling error %d: %s\n", error->status_code, error->message);
    
    Response *res = create_response(client_fd);
    
    // Set appropriate status code
    response_status(res, error->status_code);
    response_set_header(res, "Content-Type", "application/json");
    response_set_header(res, "X-Error-Handler", "custom");
    
    // Create detailed error response
    char error_response[1024];
    snprintf(error_response, sizeof(error_response),
        "{"
        "\"error\":\"%s\","
        "\"message\":\"%s\","
        "\"status\":%d,"
        "\"details\":{"
        "\"file\":\"%s\","
        "\"line\":%d,"
        "\"function\":\"%s\""
        "},"
        "\"handled_by\":\"custom_error_handler\""
        "}",
        error_get_status_text(error->code),
        error->message,
        error->status_code,
        error->file,
        error->line,
        error->function
    );
    
    response_send(res, error_response);
    destroy_response(res);
    
    // Mark error as handled
    error->is_handled = true;
}

int main() {
    App app = create_app();

    // Set up custom error handler
    app.error(&app, custom_error_handler);

    // Add logging middleware
    app.use(&app, log_handler);
    
    // GET routes
    app.get(&app, "/", hello_handler);
    app.get(&app, "/2", hello_handler2);
    app.get(&app, "/users/:id", user_handler);
    app.get(&app, "/json", json_handler);
    app.get(&app, "/status/:code", status_test_handler);  // Test different status codes
    app.get(&app, "/error/:type", error_test_handler);    // Test error handling
    
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