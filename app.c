#include "app.h"
#include "response.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

// middleware to attach Response to each request
void express_init(int client_fd, void (*next)(void *), void *context) {
    NextContext *ctx = (NextContext *)context;
    Response *res = malloc(sizeof(Response));
    response_init(res, client_fd);
    ctx->user_context = res;
    next(ctx);
    free(res);
}

void app_get(App *app, const char *path, Handler handler) {
    printf("[DEBUG] app_get: path=%s\n", path);
    router_add_layer(&app->router, "GET", path, handler);
}

void app_post(App *app, const char *path, Handler handler) {
    printf("[DEBUG] app_post: path=%s\n", path);
    router_add_layer(&app->router, "POST", path, handler);
}

void app_put(App *app, const char *path, Handler handler) {
    printf("[DEBUG] app_put: path=%s\n", path);
    router_add_layer(&app->router, "PUT", path, handler);
}

void app_delete(App *app, const char *path, Handler handler) {
    printf("[DEBUG] app_delete: path=%s\n", path);
    router_add_layer(&app->router, "DELETE", path, handler);
}

void app_patch(App *app, const char *path, Handler handler) {
    printf("[DEBUG] app_patch: path=%s\n", path);
    router_add_layer(&app->router, "PATCH", path, handler);
}

void app_options(App *app, const char *path, Handler handler) {
    printf("[DEBUG] app_options: path=%s\n", path);
    router_add_layer(&app->router, "OPTIONS", path, handler);
}

void app_use(App *app, Handler handler) {
    printf("[DEBUG] app_use: registering middleware\n");
    router_add_layer(&app->router, "USE", "/", handler);
}

void app_listen(App *app, int port) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // bind
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    // listen
    listen(server_fd, 3);
    printf("Listening on port %d\n", port);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        printf("[DEBUG] app_listen: accepted client_fd=%d\n", client_fd);

        char buffer[1024] = {0};
        read(client_fd, buffer, sizeof(buffer) -1);
        printf("[DEBUG] app_listen: received request: %s\n", buffer);

        // Create and initialize request object
        Request *req = malloc(sizeof(Request));
        request_init(req, client_fd, buffer);

        router_handle(&app->router, req->method, req->path, client_fd, req);
        
        free(req);
        close(client_fd);
    }
}

App create_app() {
    App app;
    app.router.layers = NULL;
    app.router.layer_count = 0;
    app.router.capacity = 0;
    app.get = app_get;
    app.post = app_post;
    app.put = app_put;
    app.delete = app_delete;
    app.patch = app_patch;
    app.options = app_options;
    app.listen = app_listen;
    app.use = app_use;

    // automatically register express_init middleware
    app.use(&app, express_init);
    return app;
}