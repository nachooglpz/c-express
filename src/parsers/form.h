#ifndef FORM_H
#define FORM_H

#include <stddef.h>

#define MAX_FORM_FIELDS 32
#define MAX_FIELD_NAME_SIZE 128
#define MAX_FIELD_VALUE_SIZE 2048
#define MAX_FILENAME_SIZE 256
#define MAX_CONTENT_TYPE_SIZE 128
#define MAX_BOUNDARY_SIZE 128

// Forward declaration for Request
struct Request;

// Form field types
typedef enum {
    FORM_FIELD_TEXT,
    FORM_FIELD_FILE
} FormFieldType;

// Represents a single form field (text or file)
typedef struct {
    char name[MAX_FIELD_NAME_SIZE];
    FormFieldType type;
    
    // For text fields
    char value[MAX_FIELD_VALUE_SIZE];
    
    // For file fields
    char filename[MAX_FILENAME_SIZE];
    char content_type[MAX_CONTENT_TYPE_SIZE];
    const char *file_data;  // Points to raw file data in request body
    size_t file_size;
} FormField;

// Form data container
typedef struct {
    FormField fields[MAX_FORM_FIELDS];
    int field_count;
    char boundary[MAX_BOUNDARY_SIZE];  // For multipart forms
    int parsed;
    char *error_message;
} FormData;

// ============================================================================
// FORM DATA PARSING FUNCTIONS
// ============================================================================

// Initialize form data structure
void form_data_init(FormData *form);

// Parse URL-encoded form data (application/x-www-form-urlencoded)
int parse_url_encoded_form(FormData *form, const char *body);

// Parse multipart form data (multipart/form-data)
int parse_multipart_form(FormData *form, const char *body, const char *boundary);

// Extract boundary from Content-Type header
char* extract_multipart_boundary(const char *content_type);

// ============================================================================
// FORM DATA ACCESS FUNCTIONS
// ============================================================================

// Get form field by name
FormField* form_data_get_field(FormData *form, const char *name);

// Get text field value by name
const char* form_data_get_value(FormData *form, const char *name);

// Get file field by name
FormField* form_data_get_file(FormData *form, const char *name);

// Get all form field names
const char** form_data_get_field_names(FormData *form);

// Check if field exists
int form_data_has_field(FormData *form, const char *name);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// URL decode form data values
void form_url_decode(char *dst, const char *src);

// Clean up form data
void form_data_cleanup(FormData *form);

// Get form field type as string
const char* form_field_type_name(FormFieldType type);

#endif
