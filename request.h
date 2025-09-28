#ifndef REQUEST_H
#define REQUEST_H

#include <stddef.h>

#define MAX_HEADERS 32
#define MAX_PARAMS 16
#define MAX_QUERY_PARAMS 16
#define MAX_HEADER_SIZE 256
#define MAX_PARAM_SIZE 128
#define MAX_BODY_SIZE 4096

typedef struct {
    char key[MAX_PARAM_SIZE];
    char value[MAX_PARAM_SIZE];
} KeyValue;

// Forward declaration
typedef struct Request Request;

typedef struct Request {
    int client_fd;
    char method[8];
    char path[256];
    char query_string[512];
    char body[MAX_BODY_SIZE];
    
    // HTTP headers
    KeyValue headers[MAX_HEADERS];
    int header_count;
    
    // URL parameters (e.g., :id in /users/:id)
    KeyValue params[MAX_PARAMS];
    int param_count;
    
    // Query parameters (e.g., ?name=john&age=25)
    KeyValue query[MAX_QUERY_PARAMS];
    int query_count;
    
    // Helper functions
    const char* (*get_header)(Request *req, const char *key);
    const char* (*get_param)(Request *req, const char *key);
    const char* (*get_query)(Request *req, const char *key);
} Request;

// Initialize request from raw HTTP data
void request_init(Request *req, int client_fd, const char *raw_request);

// Parse URL parameters based on route pattern
void request_parse_params(Request *req, const char *route_pattern, const char *actual_path);

// Set parameters from RouteMatch result (for advanced pattern matching)
void request_set_route_params(Request *req, void *match);

// Helper function to parse query string
void parse_query_string(const char *query_string, KeyValue *query_params, int *query_count);

// Helper function to parse HTTP headers
void parse_headers(const char *raw_request, KeyValue *headers, int *header_count);

#endif
