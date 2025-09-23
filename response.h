#ifndef RESPONSE_H
#define RESPONSE_H

#include <stddef.h>

#define MAX_RESPONSE_HEADERS 16
#define MAX_HEADER_SIZE 256

typedef struct {
    char key[MAX_HEADER_SIZE];
    char value[MAX_HEADER_SIZE];
} ResponseHeader;

// Forward declaration
typedef struct Response Response;

// response struct for each request
typedef struct Response {
    int client_fd;
    char content_type[64];
    ResponseHeader headers[MAX_RESPONSE_HEADERS];
    int header_count;
    void (*set_header)(Response *res, const char *key, const char *value);
    void (*send)(Response *res, const char *body);
    void (*json)(Response *res, const char *json_str);
} Response;

// attach send implementation to Response
void response_init(Response *res, int client_fd);

#endif
