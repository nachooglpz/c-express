#include "app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

void app_get(App *app, const char *path, void (*handler)(int)) {
    router_add_route(&app->router, path, handler);
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

        // for simplicity always call the first route's handler
        if (app->router.route_count > 0) {
            app->router.routes[0].layer.handler(client_fd);
        }
        close(client_fd);
    }
}

App create_app() {
    App app;
    app.router.routes = NULL;
    app.router.route_count = 0;
    app.router.capacity = 0;
    app.get = app_get;
    app.listen = app_listen;
    return app;
}