#include "response.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void response_set_header(Response *res, const char *key, const char *value) {
    // Check if header already exists and update it
    for (int i = 0; i < res->header_count; i++) {
        if (strcmp(res->headers[i].key, key) == 0) {
            strncpy(res->headers[i].value, value, sizeof(res->headers[i].value) - 1);
            res->headers[i].value[sizeof(res->headers[i].value) - 1] = '\0';
            return;
        }
    }
    
    // Add new header if we have space
    if (res->header_count < MAX_RESPONSE_HEADERS) {
        strncpy(res->headers[res->header_count].key, key, sizeof(res->headers[res->header_count].key) - 1);
        res->headers[res->header_count].key[sizeof(res->headers[res->header_count].key) - 1] = '\0';
        strncpy(res->headers[res->header_count].value, value, sizeof(res->headers[res->header_count].value) - 1);
        res->headers[res->header_count].value[sizeof(res->headers[res->header_count].value) - 1] = '\0';
        res->header_count++;
    }
    
    // Special handling for Content-Type (backward compatibility)
    if (strcmp(key, "Content-Type") == 0) {
        strncpy(res->content_type, value, sizeof(res->content_type) - 1);
        res->content_type[sizeof(res->content_type) - 1] = '\0';
    }
}

void response_send(Response *res, const char *body) {
    char header[1024];
    int body_len = strlen(body);
    
    // Start with basic HTTP response
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n");
    
    // Add Content-Type
    header_len += snprintf(header + header_len, sizeof(header) - header_len,
        "Content-Type: %s\r\n", 
        res->content_type[0] ? res->content_type : "text/plain");
    
    // Add custom headers
    for (int i = 0; i < res->header_count; i++) {
        // Skip Content-Type as we already added it
        if (strcmp(res->headers[i].key, "Content-Type") != 0) {
            header_len += snprintf(header + header_len, sizeof(header) - header_len,
                "%s: %s\r\n", res->headers[i].key, res->headers[i].value);
        }
    }
    
    // Add Content-Length and end headers
    header_len += snprintf(header + header_len, sizeof(header) - header_len,
        "Content-Length: %d\r\n\r\n", body_len);
    
    write(res->client_fd, header, header_len);
    write(res->client_fd, body, body_len);
}

void response_json(Response *res, const char *json_str) {
    res->set_header(res, "Content-Type", "application/json");
    res->send(res, json_str);
}

void response_init(Response *res, int client_fd) {
    res->client_fd = client_fd;
    res->content_type[0] = '\0';
    res->header_count = 0;
    res->set_header = response_set_header;
    res->send = response_send;
    res->json = response_json;
}
