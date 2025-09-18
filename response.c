#include "response.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void response_set_header(Response *res, const char *key, const char *value) {
    if (strcmp(key, "Content-Type") == 0) {
        strncpy(res->content_type, value, sizeof(res->content_type) - 1);
        res->content_type[sizeof(res->content_type) - 1] = '\0';
    }
}

void response_send(Response *res, const char *body) {
    char header[256];
    int body_len = strlen(body);
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n",
        res->content_type[0] ? res->content_type : "text/plain", body_len);
    write(res->client_fd, header, strlen(header));
    write(res->client_fd, body, body_len);
}

void response_json(Response *res, const char *json_str) {
    res->set_header(res, "Content-Type", "application/json");
    res->send(res, json_str);
}

void response_init(Response *res, int client_fd) {
    res->client_fd = client_fd;
    res->content_type[0] = '\0';
    res->set_header = response_set_header;
    res->send = response_send;
    res->json = response_json;
}
