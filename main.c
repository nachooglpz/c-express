#include "app.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// first middleware for GET /
void log_handler(int client_fd, void (*next)(void *), void *context) {
    printf("First handler: logging request\n");
    next(context); // call next middleware
}

// second middleware for GET /
void hello_handler(int client_fd, void (*next)(void *), void *context) {
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello World!\n";
    write(client_fd, response, strlen(response));
    // no next(context) here, end chain
}

// handler for GET /2
void hello_handler2(int client_fd, void (*next)(void *), void *context) {
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 15\r\n"
        "\r\n"
        "Hello World 2!\n";
    write(client_fd, response, strlen(response));
}

// handler for POST /post
void post_handler(int client_fd, void (*next)(void *), void *context) {
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 26\r\n"
        "\r\n"
        "This is the POST handler!\n";
    write(client_fd, response, strlen(response));
}

int main() {
    App app = create_app();

    // chain two handlers for GET /
    app.get(&app, "/", log_handler);
    app.get(&app, "/", hello_handler);
    app.get(&app, "/2", hello_handler2);
    app.post(&app, "/post", post_handler);
    app.listen(&app, 3000);

    free(app.router.layers);
    return 0;
}