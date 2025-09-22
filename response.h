#ifndef RESPONSE_H
#define RESPONSE_H

#include <stddef.h>

// Forward declaration
typedef struct Response Response;

// response struct for each request
typedef struct Response {
    int client_fd;
    char content_type[64];
    void (*set_header)(Response *res, const char *key, const char *value);
    void (*send)(Response *res, const char *body);
    void (*json)(Response *res, const char *json_str);
} Response;

// attach send implementation to Response
void response_init(Response *res, int client_fd);

#endif
