#include "response.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// Helper function to get status text from status code
const char* get_status_text(int status_code) {
    switch (status_code) {
        case 200: return "OK";
        case 201: return "Created";
        case 202: return "Accepted";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 409: return "Conflict";
        case 422: return "Unprocessable Entity";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        default: return "Unknown";
    }
}

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

void response_status(Response *res, int code) {
    res->status_code = code;
}

void response_send(Response *res, const char *body) {
    char header[1024];
    int body_len = strlen(body);
    
    // Start with HTTP response line using current status code
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n", res->status_code, get_status_text(res->status_code));
    
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

void response_send_status(Response *res, int code) {
    res->status_code = code;
    const char *status_text = get_status_text(code);
    res->send(res, status_text);
}

void response_init(Response *res, int client_fd) {
    res->client_fd = client_fd;
    res->status_code = 200;  // Default to 200 OK
    res->content_type[0] = '\0';
    res->header_count = 0;
    res->set_header = response_set_header;
    res->status = response_status;
    res->send = response_send;
    res->json = response_json;
    res->send_status = response_send_status;
}

Response *create_response(int client_fd) {
    Response *res = malloc(sizeof(Response));
    if (res) {
        response_init(res, client_fd);
    }
    return res;
}

void destroy_response(Response *res) {
    if (res) {
        free(res);
    }
}
