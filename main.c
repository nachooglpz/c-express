#include "app.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void hello_handler(int client_fd) {
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello World!\n";
    write(client_fd, response, strlen(response));
}

void hello_handler2(int client_fd) {
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 15\r\n"
        "\r\n"
        "Hello World 2!\n";
    write(client_fd, response, strlen(response));
}

void post_handler(int client_fd) {
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

    app.get(&app, "/", hello_handler);
    app.get(&app, "/2", hello_handler2);
    app.post(&app, "/post", post_handler);
    app.listen(&app, 3000);

    free(app.router.layers);

    return 0;
}