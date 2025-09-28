#ifndef REQUEST_H
#define REQUEST_H

#include <stddef.h>
#include "../parsers/json.h"
#include "../parsers/form.h"

#define MAX_HEADERS 32
#define MAX_PARAMS 16
#define MAX_QUERY_PARAMS 16
#define MAX_HEADER_SIZE 256
#define MAX_PARAM_SIZE 128
#define MAX_BODY_SIZE 16384

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
    
    // JSON parsing support
    JsonValue *parsed_json;
    int json_parsed;
    char *json_error;
    
    // Form data parsing support
    FormData form_data;
    int form_parsed;
    
    // Helper functions
    const char* (*get_header)(Request *req, const char *key);
    const char* (*get_param)(Request *req, const char *key);
    const char* (*get_query)(Request *req, const char *key);
    
    // JSON helper functions
    JsonValue* (*get_json)(Request *req);
    const char* (*get_json_string)(Request *req, const char *key);
    double (*get_json_number)(Request *req, const char *key);
    int (*get_json_bool)(Request *req, const char *key);
    JsonObject* (*get_json_object)(Request *req, const char *key);
    JsonArray* (*get_json_array)(Request *req, const char *key);
    int (*validate_json_schema)(Request *req, JsonSchema *schema);
    
    // Form data helper functions
    FormData* (*get_form)(Request *req);
    const char* (*get_form_value)(Request *req, const char *name);
    FormField* (*get_form_field)(Request *req, const char *name);
    FormField* (*get_form_file)(Request *req, const char *name);
    int (*has_form_field)(Request *req, const char *name);
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

// JSON request body functions
JsonValue* request_get_json(Request *req);
const char* request_get_json_string(Request *req, const char *key);
double request_get_json_number(Request *req, const char *key);
int request_get_json_bool(Request *req, const char *key);
JsonObject* request_get_json_object(Request *req, const char *key);
JsonArray* request_get_json_array(Request *req, const char *key);
int request_validate_json_schema(Request *req, JsonSchema *schema);
void request_free_json(Request *req);

// Content type detection
const char* request_get_content_type(Request *req);
int request_is_json(Request *req);

// Form data request body functions
FormData* request_get_form(Request *req);
const char* request_get_form_value(Request *req, const char *name);
FormField* request_get_form_field(Request *req, const char *name);
FormField* request_get_form_file(Request *req, const char *name);
int request_has_form_field(Request *req, const char *name);
int request_is_form_data(Request *req);
int request_is_multipart_form(Request *req);
void request_free_form(Request *req);

#endif
