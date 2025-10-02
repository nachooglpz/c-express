#define _GNU_SOURCE
#include "request.h"
#include "../debug.h"
#include "../core/route.h"
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
    if (!path || (size_t)*pos >= strlen(path)) return NULL;
    
    // Skip slashes
    while (path[*pos] == '/') (*pos)++;
    if ((size_t)*pos >= strlen(path)) return NULL;
    
    int start = *pos;
    // Find end of segment
    while ((size_t)*pos < strlen(path) && path[*pos] != '/') (*pos)++;
    
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
    
    // Initialize JSON fields
    req->parsed_json = NULL;
    req->json_parsed = 0;
    req->json_error = NULL;
    
    // Initialize form data fields
    form_data_init(&req->form_data);
    req->form_parsed = 0;
    
    // Initialize streaming fields
    req->stream = NULL;
    req->body_streamed = 0;
    req->body_complete = 0;
    
    // Set function pointers
    req->get_header = request_get_header;
    req->get_param = request_get_param;
    req->get_query = request_get_query;
    req->get_json = request_get_json;
    req->get_json_string = request_get_json_string;
    req->get_json_number = request_get_json_number;
    req->get_json_bool = request_get_json_bool;
    req->get_json_object = request_get_json_object;
    req->get_json_array = request_get_json_array;
    req->validate_json_schema = request_validate_json_schema;
    
    // Set form data function pointers
    req->get_form = request_get_form;
    req->get_form_value = request_get_form_value;
    req->get_form_field = request_get_form_field;
    req->get_form_file = request_get_form_file;
    req->has_form_field = request_has_form_field;
    
    // Set streaming function pointers
    req->is_body_streamed = request_is_body_streamed;
    req->is_streaming_complete = request_is_streaming_complete;
    req->get_stream = request_get_stream;
    req->get_body_content = request_get_body_content;
    req->get_temp_file = request_get_temp_file;
    req->save_body_to_file = request_save_body_to_file;
    req->get_body_size = request_get_body_size;
    
    // Parse the first line to get method and path
    char first_line[512];
    const char *line_end = strchr(raw_request, '\r');
    if (!line_end) line_end = strchr(raw_request, '\n');
    if (!line_end) return;
    
    int line_len = line_end - raw_request;
    if ((size_t)line_len >= sizeof(first_line)) line_len = sizeof(first_line) - 1;
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
    
    // Parse body (handle both \r\n\r\n and \n\n patterns)
    DEBUG_PRINT("Looking for body separator in raw_request (first 200 chars): %.200s\n", raw_request);
    const char *body_start = strstr(raw_request, "\r\n\r\n");
    if (body_start) {
        body_start += 4; // Skip \r\n\r\n
        DEBUG_PRINT_STR("Found \\r\\n\\r\\n separator\n");
    } else {
        body_start = strstr(raw_request, "\n\n");
        if (body_start) {
            body_start += 2; // Skip \n\n
            DEBUG_PRINT_STR("Found \\n\\n separator\n");
        } else {
            DEBUG_PRINT_STR("No body separator found\n");
        }
    }
    
    if (body_start) {
        size_t body_len = strlen(body_start);
        size_t max_copy = (body_len < MAX_BODY_SIZE - 1) ? body_len : MAX_BODY_SIZE - 1;
        strncpy(req->body, body_start, max_copy);
        req->body[max_copy] = '\0';
        DEBUG_PRINT("Parsed body length: %zu (original: %zu)\n", strlen(req->body), body_len);
        DEBUG_PRINT("Body first 100 chars: %.100s\n", req->body);
    } else {
        req->body[0] = '\0';
        DEBUG_PRINT_STR("No body found in request\n");
    }
    
    DEBUG_PRINT("request_init: method=%s, path=%s, query=%s\n", 
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
        
        DEBUG_PRINT("request_set_route_params: set param '%s' = '%s'\n", 
               req->params[i].key, req->params[i].value);
    }
    
    DEBUG_PRINT("request_set_route_params: set %d parameters\n", req->param_count);
}

// Initialize request with streaming support (headers already parsed)
void request_init_streaming(Request *req, int client_fd, const char *headers_only) {
    // First initialize normally but with empty body
    char empty_request[4096];
    snprintf(empty_request, sizeof(empty_request), "%s\r\n\r\n", headers_only);
    request_init(req, client_fd, empty_request);
    
    // Clear the body since we'll handle it via streaming
    req->body[0] = '\0';
    
    // Get content length and transfer encoding headers
    const char *content_length = req->get_header(req, "Content-Length");
    const char *transfer_encoding = req->get_header(req, "Transfer-Encoding");
    
    // Create stream context
    req->stream = stream_create(client_fd, content_length, transfer_encoding);
    if (req->stream) {
        req->body_streamed = 1;
        req->body_complete = 0;
        DEBUG_PRINT_STR("request_init_streaming: Created stream context\n");
    } else {
        DEBUG_PRINT_STR("request_init_streaming: Failed to create stream context\n");
        req->body_streamed = 0;
        req->body_complete = 1;
    }
}

// ============================================================================
// JSON REQUEST BODY PARSING
// ============================================================================

// Get content type from request headers
const char* request_get_content_type(Request *req) {
    return request_get_header(req, "Content-Type");
}

// Check if request contains JSON content
int request_is_json(Request *req) {
    const char *content_type = request_get_content_type(req);
    if (!content_type) return 0;
    
    return strstr(content_type, "application/json") != NULL;
}

// Parse JSON from request body (lazy parsing)
JsonValue* request_get_json(Request *req) {
    if (!req) return NULL;
    
    // Return cached result if already parsed
    if (req->json_parsed) {
        return req->parsed_json;
    }
    
    // Mark as parsed to avoid re-parsing
    req->json_parsed = 1;
    
    // Check if content type is JSON
    if (!request_is_json(req)) {
        req->json_error = strdup("Content-Type is not application/json");
        return NULL;
    }
    
    // Check if body is empty
    if (!req->body[0]) {
        req->json_error = strdup("Request body is empty");
        return NULL;
    }
    
    // Parse JSON
    char *error_message = NULL;
    req->parsed_json = json_parse_with_error(req->body, &error_message);
    
    if (error_message) {
        req->json_error = error_message;
        return NULL;
    }
    
    DEBUG_PRINT_STR("request_get_json: successfully parsed JSON from request body\n");
    return req->parsed_json;
}

// Get string value from JSON object in request
const char* request_get_json_string(Request *req, const char *key) {
    JsonValue *json = request_get_json(req);
    if (!json || json->type != JSON_OBJECT || !key) return NULL;
    
    return json_object_get_string(json->data.object_value, key);
}

// Get number value from JSON object in request  
double request_get_json_number(Request *req, const char *key) {
    JsonValue *json = request_get_json(req);
    if (!json || json->type != JSON_OBJECT || !key) return 0.0;
    
    return json_object_get_number(json->data.object_value, key);
}

// Get boolean value from JSON object in request
int request_get_json_bool(Request *req, const char *key) {
    JsonValue *json = request_get_json(req);
    if (!json || json->type != JSON_OBJECT || !key) return 0;
    
    return json_object_get_bool(json->data.object_value, key);
}

// Get nested object from JSON object in request
JsonObject* request_get_json_object(Request *req, const char *key) {
    JsonValue *json = request_get_json(req);
    if (!json || json->type != JSON_OBJECT || !key) return NULL;
    
    return json_object_get_object(json->data.object_value, key);
}

// Get array from JSON object in request
JsonArray* request_get_json_array(Request *req, const char *key) {
    JsonValue *json = request_get_json(req);
    if (!json || json->type != JSON_OBJECT || !key) return NULL;
    
    return json_object_get_array(json->data.object_value, key);
}

// Validate JSON against schema
int request_validate_json_schema(Request *req, JsonSchema *schema) {
    JsonValue *json = request_get_json(req);
    if (!json || !schema) return 0;
    
    JsonValidationResult *result = json_validate_schema(json, schema);
    if (!result) return 0;
    
    int is_valid = result->is_valid;
    
    // Print validation errors for debugging
    if (!is_valid) {
        DEBUG_PRINT("JSON validation failed for schema '%s':\n", 
               schema->schema_name ? schema->schema_name : "unnamed");
        for (int i = 0; i < result->error_count; i++) {
            JsonValidationError *error = &result->errors[i];
            (void)error; // Mark as potentially unused when DEBUG is disabled
            DEBUG_PRINT("  - %s: %s (expected %s, got %s)\n",
                   error->field_path,
                   error->error_message,
                   json_type_name(error->expected_type),
                   json_type_name(error->actual_type));
        }
    } else {
        DEBUG_PRINT("JSON validation passed for schema '%s'\n",
               schema->schema_name ? schema->schema_name : "unnamed");
    }
    
    json_free_validation_result(result);
    return is_valid;
}

// Free JSON parsing resources
void request_free_json(Request *req) {
    if (!req) return;
    
    if (req->parsed_json) {
        json_free_value(req->parsed_json);
        req->parsed_json = NULL;
    }
    
    if (req->json_error) {
        free(req->json_error);
        req->json_error = NULL;
    }
    
    req->json_parsed = 0;
}

// ============================================================================
// FORM DATA REQUEST BODY PARSING
// ============================================================================

// Check if request contains form data
int request_is_form_data(Request *req) {
    const char *content_type = request_get_content_type(req);
    if (!content_type) return 0;
    
    return strstr(content_type, "application/x-www-form-urlencoded") != NULL;
}

// Check if request contains multipart form data
int request_is_multipart_form(Request *req) {
    const char *content_type = request_get_content_type(req);
    if (!content_type) return 0;
    
    return strstr(content_type, "multipart/form-data") != NULL;
}

// Parse form data from request body (lazy parsing)
FormData* request_get_form(Request *req) {
    if (!req) return NULL;
    
    // Return cached result if already parsed
    if (req->form_parsed) {
        return &req->form_data;
    }
    
    // Mark as parsed to avoid re-parsing
    req->form_parsed = 1;
    
    DEBUG_PRINT_STR("Parsing form data from request body\n");
    
    // Check if body is empty
    if (!req->body[0]) {
        req->form_data.error_message = strdup("Request body is empty");
        return NULL;
    }
    
    // Parse based on content type
    if (request_is_multipart_form(req)) {
        // Parse multipart form data
        const char *content_type = request_get_content_type(req);
        char *boundary = extract_multipart_boundary(content_type);
        
        if (!boundary) {
            req->form_data.error_message = strdup("Could not extract multipart boundary");
            return NULL;
        }
        
        DEBUG_PRINT("Parsing multipart form data with boundary: %s\n", boundary);
        
        int success = parse_multipart_form(&req->form_data, req->body, boundary);
        free(boundary);
        
        if (!success) {
            return NULL;
        }
    } else if (request_is_form_data(req)) {
        // Parse URL-encoded form data
        DEBUG_PRINT_STR("Parsing URL-encoded form data\n");
        
        int success = parse_url_encoded_form(&req->form_data, req->body);
        if (!success) {
            req->form_data.error_message = strdup("Failed to parse URL-encoded form data");
            return NULL;
        }
    } else {
        req->form_data.error_message = strdup("Content-Type is not a supported form data type");
        return NULL;
    }
    
    DEBUG_PRINT("Successfully parsed form data with %d fields\n", req->form_data.field_count);
    return &req->form_data;
}

// Get form field value by name
const char* request_get_form_value(Request *req, const char *name) {
    FormData *form = request_get_form(req);
    if (!form) return NULL;
    
    return form_data_get_value(form, name);
}

// Get form field by name
FormField* request_get_form_field(Request *req, const char *name) {
    FormData *form = request_get_form(req);
    if (!form) return NULL;
    
    return form_data_get_field(form, name);
}

// Get form file field by name
FormField* request_get_form_file(Request *req, const char *name) {
    FormData *form = request_get_form(req);
    if (!form) return NULL;
    
    return form_data_get_file(form, name);
}

// Check if form field exists
int request_has_form_field(Request *req, const char *name) {
    FormData *form = request_get_form(req);
    if (!form) return 0;
    
    return form_data_has_field(form, name);
}

// Free form data parsing resources
void request_free_form(Request *req) {
    if (!req) return;
    
    form_data_cleanup(&req->form_data);
    req->form_parsed = 0;
}

// Streaming request body functions
int request_is_body_streamed(Request *req) {
    return req ? req->body_streamed : 0;
}

int request_is_streaming_complete(Request *req) {
    if (!req || !req->stream) return 1;
    return stream_is_complete(req->stream);
}

StreamContext* request_get_stream(Request *req) {
    return req ? req->stream : NULL;
}

const char* request_get_body_content(Request *req) {
    if (!req || !req->stream) {
        // Fallback to legacy body for backward compatibility
        return req ? req->body : NULL;
    }
    
    if (req->body_streamed) {
        return stream_get_memory_content(req->stream);
    }
    
    return req->body;
}

const char* request_get_temp_file(Request *req) {
    if (!req || !req->stream) return NULL;
    return stream_get_temp_file(req->stream);
}

int request_save_body_to_file(Request *req, const char *filename) {
    if (!req || !req->stream) return -1;
    return stream_save_to_file(req->stream, filename);
}

size_t request_get_body_size(Request *req) {
    if (!req) return 0;
    
    if (req->stream) {
        return stream_get_content_length(req->stream);
    }
    
    // Fallback to legacy body
    return strlen(req->body);
}

void request_free_stream(Request *req) {
    if (!req || !req->stream) return;
    
    stream_destroy(req->stream);
    req->stream = NULL;
    req->body_streamed = 0;
    req->body_complete = 0;
}

// Comprehensive cleanup function that properly frees all allocated memory
void request_destroy(Request *req) {
    if (!req) return;
    
    // Free JSON-related allocations
    request_free_json(req);
    
    // Free form-related allocations  
    request_free_form(req);
    
    // Free streaming-related allocations
    request_free_stream(req);
    
    // Note: The Request struct itself should be freed by the caller
    // since it might be stack-allocated or part of a larger structure
}
