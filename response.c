#include "response.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

// implementation of send
void response_send(Response *res, const char *body) {
    // for now, just print to console and send to client
    printf("[DEBUG] Response set: %s\n", body);
    char header[256];
    int body_len = strlen(body);
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n", body_len);
    write(res->client_fd, header, strlen(header));
    write(res->client_fd, body, body_len);
}

void response_init(Response *res, int client_fd) {
    res->client_fd = client_fd;
    res->send = response_send;
}
