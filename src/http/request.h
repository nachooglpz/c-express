#ifndef REQUEST_H
#define REQUEST_H

#include <stddef.h>
#include "../parsers/json.h"
#include "../parsers/form.h"
#include "streaming.h"

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

// Forward declaration for self-referencing pointers
struct Request;

struct Request {
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
    
    // Streaming support for large bodies
    StreamContext *stream;
    int body_streamed;          // 1 if body is being streamed
    int body_complete;          // 1 if streaming is complete
    
    // Helper functions
    const char* (*get_header)(struct Request *req, const char *key);
    const char* (*get_param)(struct Request *req, const char *key);
    const char* (*get_query)(struct Request *req, const char *key);
    
    // JSON helper functions
    JsonValue* (*get_json)(struct Request *req);
    const char* (*get_json_string)(struct Request *req, const char *key);
    double (*get_json_number)(struct Request *req, const char *key);
    int (*get_json_bool)(struct Request *req, const char *key);
    JsonObject* (*get_json_object)(struct Request *req, const char *key);
    JsonArray* (*get_json_array)(struct Request *req, const char *key);
    int (*validate_json_schema)(struct Request *req, JsonSchema *schema);
    
    // Form data helper functions
    FormData* (*get_form)(struct Request *req);
    const char* (*get_form_value)(struct Request *req, const char *name);
    FormField* (*get_form_field)(struct Request *req, const char *name);
    FormField* (*get_form_file)(struct Request *req, const char *name);
    int (*has_form_field)(struct Request *req, const char *name);
    
    // Streaming functions
    int (*is_body_streamed)(struct Request *req);
    int (*is_streaming_complete)(struct Request *req);
    StreamContext* (*get_stream)(struct Request *req);
    const char* (*get_body_content)(struct Request *req);  // For small bodies
    const char* (*get_temp_file)(struct Request *req);     // For large bodies
    int (*save_body_to_file)(struct Request *req, const char *filename);
    size_t (*get_body_size)(struct Request *req);
};

typedef struct Request Request;

// Initialize request from raw HTTP data
void request_init(struct Request *req, int client_fd, const char *raw_request);

// Initialize request with streaming support
void request_init_streaming(struct Request *req, int client_fd, const char *headers_only);

// Parse URL parameters based on route pattern
void request_parse_params(struct Request *req, const char *route_pattern, const char *actual_path);

// Set parameters from RouteMatch result (for advanced pattern matching)
void request_set_route_params(struct Request *req, void *match);

// Helper function to parse query string
void parse_query_string(const char *query_string, KeyValue *query_params, int *query_count);

// Helper function to parse HTTP headers
void parse_headers(const char *raw_request, KeyValue *headers, int *header_count);

// JSON request body functions
JsonValue* request_get_json(struct Request *req);
const char* request_get_json_string(struct Request *req, const char *key);
double request_get_json_number(struct Request *req, const char *key);
int request_get_json_bool(struct Request *req, const char *key);
JsonObject* request_get_json_object(struct Request *req, const char *key);
JsonArray* request_get_json_array(struct Request *req, const char *key);
int request_validate_json_schema(struct Request *req, JsonSchema *schema);
void request_free_json(struct Request *req);

// Content type detection
const char* request_get_content_type(struct Request *req);
int request_is_json(struct Request *req);

// Form data request body functions
FormData* request_get_form(struct Request *req);
const char* request_get_form_value(struct Request *req, const char *name);
FormField* request_get_form_field(struct Request *req, const char *name);
FormField* request_get_form_file(struct Request *req, const char *name);
int request_has_form_field(struct Request *req, const char *name);

// Streaming request body functions
int request_is_body_streamed(struct Request *req);
int request_is_streaming_complete(struct Request *req);
StreamContext* request_get_stream(struct Request *req);
const char* request_get_body_content(struct Request *req);
const char* request_get_temp_file(struct Request *req);
int request_save_body_to_file(struct Request *req, const char *filename);
size_t request_get_body_size(struct Request *req);
void request_free_stream(struct Request *req);

// Legacy form functions
int request_is_form_data(struct Request *req);
int request_is_multipart_form(struct Request *req);
void request_free_form(struct Request *req);

#endif
