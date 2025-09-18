#ifndef RESPONSE_H
#define RESPONSE_H

#include <stddef.h>

// response struct for each request
typedef struct {
    int client_fd;
    char content_type[64];
    void (*set_header)(struct Response *res, const char *key, const char *value);
    void (*send)(struct Response *res, const char *body);
    void (*json)(struct Response *res, const char *json_str);
} Response;

// attach send implementation to Response
void response_init(Response *res, int client_fd);

#endif
