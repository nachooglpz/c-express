#include "app.h"
#include "response.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// first middleware for GET /
void log_handler(int client_fd, void (*next)(void *), void *context) {
    printf("First handler: logging request\n");
    NextContext *ctx = (NextContext *)context;
   next(ctx);
}

// second middleware for GET /
void hello_handler(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = (Response *)ctx->user_context;
    printf("[DEBUG] hello_handler: sending the response");
    res->send(res, "Hello World!");
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
    res->json(res, "{\"hello\":\"world\"}");
}

// handler for POST /post
void post_handler(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = (Response *)ctx->user_context;
    printf("[DEBUG] post_handler: sending response\n");
    res->send(res, "This is the POST handler!");
}

int main() {
    App app = create_app();

    app.get(&app, "/", log_handler);
    app.get(&app, "/", hello_handler);
    app.get(&app, "/2", hello_handler2);
    app.post(&app, "/post", post_handler);
    app.get(&app, "/json", json_handler);
    app.listen(&app, 3000);

    free(app.router.layers);
    return 0;
}