#include "request.h"
#include "route.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Helper function to get header value
const char* request_get_header(Request *req, const char *key) {
    for (int i = 0; i < req->header_count; i++) {
        if (strcasecmp(req->headers[i].key, key) == 0) {
            return req->headers[i].value;
        }
    }
    return NULL;
}

// Helper function to get URL parameter value
const char* request_get_param(Request *req, const char *key) {
    for (int i = 0; i < req->param_count; i++) {
        if (strcmp(req->params[i].key, key) == 0) {
            return req->params[i].value;
        }
    }
    return NULL;
}

// Helper function to get query parameter value
const char* request_get_query(Request *req, const char *key) {
    for (int i = 0; i < req->query_count; i++) {
        if (strcmp(req->query[i].key, key) == 0) {
            return req->query[i].value;
        }
    }
    return NULL;
}

// URL decode helper function
void url_decode(char *dst, const char *src) {
    char *p = dst;
    char hex[3];
    
    while (*src) {
        if (*src == '%' && *(src + 1) && *(src + 2)) {
            hex[0] = *(src + 1);
            hex[1] = *(src + 2);
            hex[2] = '\0';
            *p++ = (char)strtol(hex, NULL, 16);
            src += 3;
        } else if (*src == '+') {
            *p++ = ' ';
            src++;
        } else {
            *p++ = *src++;
        }
    }
    *p = '\0';
}

// Parse query string into key-value pairs
void parse_query_string(const char *query_string, KeyValue *query_params, int *query_count) {
    *query_count = 0;
    if (!query_string || strlen(query_string) == 0) {
        return;
    }
    
    char *query_copy = strdup(query_string);
    char *pair = strtok(query_copy, "&");
    
    while (pair && *query_count < MAX_QUERY_PARAMS) {
        char *equals = strchr(pair, '=');
        if (equals) {
            *equals = '\0';
            url_decode(query_params[*query_count].key, pair);
            url_decode(query_params[*query_count].value, equals + 1);
        } else {
            url_decode(query_params[*query_count].key, pair);
            query_params[*query_count].value[0] = '\0';
        }
        (*query_count)++;
        pair = strtok(NULL, "&");
    }
    
    free(query_copy);
}

// Parse HTTP headers from raw request
void parse_headers(const char *raw_request, KeyValue *headers, int *header_count) {
    *header_count = 0;
    
    // Skip the first line (HTTP method line)
    const char *line_start = strchr(raw_request, '\n');
    if (!line_start) return;
    line_start++; // Skip the newline
    
    while (line_start && *line_start && *header_count < MAX_HEADERS) {
        const char *line_end = strchr(line_start, '\r');
        if (!line_end) line_end = strchr(line_start, '\n');
        if (!line_end) break;
        
        // Empty line indicates end of headers
        if (line_end == line_start || (line_end == line_start + 1 && *line_start == '\r')) {
            break;
        }
        
        // Find the colon separator
        const char *colon = strchr(line_start, ':');
        if (colon && colon < line_end) {
            int key_len = colon - line_start;
            const char *value_start = colon + 1;
            
            // Skip whitespace after colon
            while (value_start < line_end && (*value_start == ' ' || *value_start == '\t')) {
                value_start++;
            }
            
            int value_len = line_end - value_start;
            
            if (key_len < MAX_PARAM_SIZE && value_len < MAX_PARAM_SIZE) {
                strncpy(headers[*header_count].key, line_start, key_len);
                headers[*header_count].key[key_len] = '\0';
                strncpy(headers[*header_count].value, value_start, value_len);
                headers[*header_count].value[value_len] = '\0';
                (*header_count)++;
            }
        }
        
        // Move to next line
        line_start = line_end;
        if (*line_start == '\r') line_start++;
        if (*line_start == '\n') line_start++;
    }
}

// Helper function to get next segment from a path (same as in layer.c)
static const char* get_next_segment_req(const char *path, int *pos, char *buffer, int buffer_size) {
    if (!path || *pos >= strlen(path)) return NULL;
    
    // Skip slashes
    while (path[*pos] == '/') (*pos)++;
    if (*pos >= strlen(path)) return NULL;
    
    int start = *pos;
    // Find end of segment
    while (*pos < strlen(path) && path[*pos] != '/') (*pos)++;
    
    int length = *pos - start;
    if (length >= buffer_size) length = buffer_size - 1;
    
    strncpy(buffer, path + start, length);
    buffer[length] = '\0';
    
    return buffer;
}

// Parse URL parameters based on route pattern (e.g., /users/:id)
void request_parse_params(Request *req, const char *route_pattern, const char *actual_path) {
    req->param_count = 0;
    
    if (!route_pattern || !actual_path) return;
    
    int pattern_pos = 0, path_pos = 0;
    char pattern_segment[64], path_segment[64];
    
    while (req->param_count < MAX_PARAMS) {
        const char *p_seg = get_next_segment_req(route_pattern, &pattern_pos, pattern_segment, sizeof(pattern_segment));
        const char *path_seg = get_next_segment_req(actual_path, &path_pos, path_segment, sizeof(path_segment));
        
        // Both exhausted - done
        if (!p_seg && !path_seg) break;
        
        // One exhausted but not the other - shouldn't happen if route matched
        if (!p_seg || !path_seg) break;
        
        if (p_seg[0] == ':') {
            // This is a parameter
            strncpy(req->params[req->param_count].key, p_seg + 1, MAX_PARAM_SIZE - 1);
            req->params[req->param_count].key[MAX_PARAM_SIZE - 1] = '\0';
            
            url_decode(req->params[req->param_count].value, path_seg);
            req->param_count++;
        }
    }
}

// Initialize request from raw HTTP data
void request_init(Request *req, int client_fd, const char *raw_request) {
    req->client_fd = client_fd;
    req->header_count = 0;
    req->param_count = 0;
    req->query_count = 0;
    
    // Set function pointers
    req->get_header = request_get_header;
    req->get_param = request_get_param;
    req->get_query = request_get_query;
    
    // Parse the first line to get method and path
    char first_line[512];
    const char *line_end = strchr(raw_request, '\r');
    if (!line_end) line_end = strchr(raw_request, '\n');
    if (!line_end) return;
    
    int line_len = line_end - raw_request;
    if (line_len >= sizeof(first_line)) line_len = sizeof(first_line) - 1;
    strncpy(first_line, raw_request, line_len);
    first_line[line_len] = '\0';
    
    // Parse method, path, and query string
    char full_path[512];
    sscanf(first_line, "%7s %511s", req->method, full_path);
    
    // Split path and query string
    char *query_start = strchr(full_path, '?');
    if (query_start) {
        *query_start = '\0';
        strncpy(req->query_string, query_start + 1, sizeof(req->query_string) - 1);
        req->query_string[sizeof(req->query_string) - 1] = '\0';
        parse_query_string(req->query_string, req->query, &req->query_count);
    } else {
        req->query_string[0] = '\0';
    }
    
    strncpy(req->path, full_path, sizeof(req->path) - 1);
    req->path[sizeof(req->path) - 1] = '\0';
    
    // Parse headers
    parse_headers(raw_request, req->headers, &req->header_count);
    
    // Parse body (simplified - assumes body starts after empty line)
    const char *body_start = strstr(raw_request, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
        strncpy(req->body, body_start, MAX_BODY_SIZE - 1);
        req->body[MAX_BODY_SIZE - 1] = '\0';
    } else {
        req->body[0] = '\0';
    }
    
    printf("[DEBUG] request_init: method=%s, path=%s, query=%s\n", 
           req->method, req->path, req->query_string);
}

// Set parameters from RouteMatch result (for advanced pattern matching)
void request_set_route_params(Request *req, void *match_ptr) {
    RouteMatch *match = (RouteMatch*)match_ptr;
    if (!req || !match) return;
    
    // Clear existing parameters
    req->param_count = 0;
    
    // Copy parameters from match result
    for (int i = 0; i < match->param_count && i < MAX_PARAMS; i++) {
        strncpy(req->params[i].key, match->params[i].name, sizeof(req->params[i].key) - 1);
        req->params[i].key[sizeof(req->params[i].key) - 1] = '\0';
        
        strncpy(req->params[i].value, match->params[i].value, sizeof(req->params[i].value) - 1);
        req->params[i].value[sizeof(req->params[i].value) - 1] = '\0';
        
        req->param_count++;
        
        printf("[DEBUG] request_set_route_params: set param '%s' = '%s'\n", 
               req->params[i].key, req->params[i].value);
    }
    
    printf("[DEBUG] request_set_route_params: set %d parameters\n", req->param_count);
}
