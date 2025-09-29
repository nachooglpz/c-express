#ifndef NEGOTIATION_H
#define NEGOTIATION_H

#include "request.h"
#include "response.h"

#define MAX_ACCEPT_TYPES 16
// Use existing MAX_CONTENT_TYPE_SIZE from form.h (avoid redefinition)

// Media type with quality value
typedef struct {
    char media_type[128];  // Use same size as form.h to avoid conflicts
    char subtype[128];     
    char main_type[128];   
    float quality;                           // q-value (0.0 to 1.0)
    int wildcard_main;                       // 1 if main type is *
    int wildcard_sub;                        // 1 if subtype is *
} AcceptType;

// Content negotiation context
typedef struct {
    AcceptType accept_types[MAX_ACCEPT_TYPES];
    int accept_count;
    char preferred_language[64];
    char preferred_encoding[64];
    char preferred_charset[64];
} ContentNegotiation;

// Content type definitions for common formats
typedef enum {
    CONTENT_JSON = 0,
    CONTENT_XML,
    CONTENT_HTML,
    CONTENT_TEXT,
    CONTENT_CSV,
    CONTENT_YAML,
    CONTENT_FORM_URLENCODED,
    CONTENT_MULTIPART,
    CONTENT_BINARY,
    CONTENT_UNKNOWN
} ContentType;

// Content format structure
typedef struct {
    ContentType type;
    const char *mime_type;
    const char *extension;
    const char *description;
} ContentFormat;

// Function declarations

// Parse Accept header and build negotiation context
ContentNegotiation* parse_accept_header(Request *req);

// Find best matching content type from available options
const char* negotiate_content_type(ContentNegotiation *negotiation, 
                                  const char **available_types, int count);

// Get content type enum from MIME type string
ContentType get_content_type_enum(const char *mime_type);

// Get MIME type string from content type enum
const char* get_mime_type_string(ContentType type);

// Check if client accepts specific content type
int accepts_content_type(ContentNegotiation *negotiation, const char *mime_type);

// Get quality value for specific content type
float get_content_type_quality(ContentNegotiation *negotiation, const char *mime_type);

// Helper functions for specific content types
int accepts_json(Request *req);
int accepts_html(Request *req);
int accepts_xml(Request *req);
int accepts_text(Request *req);

// Enhanced response methods with content negotiation
void response_negotiate_and_send(Response *res, Request *req, 
                                const char *json_data, const char *html_data, 
                                const char *text_data);

// Format data based on negotiated content type
char* format_response_data(ContentType type, const char *data);

// Cleanup
void free_content_negotiation(ContentNegotiation *negotiation);

// Utility functions for debugging
void print_content_negotiation(ContentNegotiation *negotiation);
const char* content_type_to_string(ContentType type);

#endif
