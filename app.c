#include "app.h"
#include "response.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

// middleware to attach Response to each request
void express_init(int client_fd, void (*next)(void *), void *context) {
    Response *res = malloc(sizeof(Response));
    response_init(res, client_fd);
    next(res);
    free(res);
}

void app_get(App *app, const char *path, Handler handler) {
    router_add_layer(&app->router, "GET", path, handler);
}

void app_post(App *app, const char *path, Handler handler) {
    router_add_layer(&app->router, "POST", path, handler);
}

void app_use(App *app, Handler handler) {
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

        char buffer[1024] = {0};
        read(client_fd, buffer, sizeof(buffer) -1);

        // parse method and path
        char method[8], path[256];
        sscanf(buffer, "%7s %255s", method, path);

        router_handle(&app->router, method, path, client_fd);
        
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
    app.listen = app_listen;
    app.use = app_use;

    // automatically register express_init middleware
    app.use(&app, express_init);
    return app;
}