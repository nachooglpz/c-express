#ifndef RESPONSE_H
#define RESPONSE_H

#include <stddef.h>

// response struct for each request
typedef struct {
    int client_fd;
    void (*send)(struct Response *res, const char *body);
} Response;

// attach send implementation to Response
void response_init(Response *res, int client_fd);

#endif
