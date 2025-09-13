#include "app.h"
#include <unistd.h>
#include <string.h>

void hello_handler(int client_fd) {
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 12\r\n"
        "\r\n"
        "Hello World!\n\n";
    write(client_fd, response, strlen(response));
}

int main() {
    App app = create_app();

    app.get(&app, "/", hello_handler);
    app.listen(&app, 3000);

    free(app.router.routes);

    return 0;
}