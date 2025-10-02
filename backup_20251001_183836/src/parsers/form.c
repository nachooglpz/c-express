#define _GNU_SOURCE
#include "form.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ============================================================================
// FORM DATA INITIALIZATION
// ============================================================================

void form_data_init(FormData *form) {
    if (!form) return;
    
    form->field_count = 0;
    form->boundary[0] = '\0';
    form->parsed = 0;
    form->error_message = NULL;
    
    // Initialize all fields
    for (int i = 0; i < MAX_FORM_FIELDS; i++) {
        form->fields[i].name[0] = '\0';
        form->fields[i].type = FORM_FIELD_TEXT;
        form->fields[i].value[0] = '\0';
        form->fields[i].filename[0] = '\0';
        form->fields[i].content_type[0] = '\0';
        form->fields[i].file_data = NULL;
        form->fields[i].file_size = 0;
    }
}

// ============================================================================
// URL-ENCODED FORM DATA PARSING
// ============================================================================

int parse_url_encoded_form(FormData *form, const char *body) {
    if (!form || !body) {
        return 0;
    }
    
    form_data_init(form);
    
    printf("[DEBUG] Parsing URL-encoded form data: %.100s\n", body);
    
    char *body_copy = strdup(body);
    char *pair = strtok(body_copy, "&");
    
    while (pair && form->field_count < MAX_FORM_FIELDS) {
        char *equals = strchr(pair, '=');
        if (equals) {
            *equals = '\0';
            
            FormField *field = &form->fields[form->field_count];
            
            // URL decode field name and value
            form_url_decode(field->name, pair);
            form_url_decode(field->value, equals + 1);
            
            field->type = FORM_FIELD_TEXT;
            form->field_count++;
            
            printf("[DEBUG] Parsed form field: '%s' = '%.50s'\n", 
                   field->name, field->value);
        } else {
            // Field without value
            FormField *field = &form->fields[form->field_count];
            form_url_decode(field->name, pair);
            field->value[0] = '\0';
            field->type = FORM_FIELD_TEXT;
            form->field_count++;
            
            printf("[DEBUG] Parsed form field (no value): '%s'\n", field->name);
        }
        
        pair = strtok(NULL, "&");
    }
    
    free(body_copy);
    form->parsed = 1;
    
    printf("[DEBUG] Parsed %d form fields from URL-encoded data\n", form->field_count);
    return 1;
}

// ============================================================================
// MULTIPART FORM DATA PARSING  
// ============================================================================

char* extract_multipart_boundary(const char *content_type) {
    if (!content_type) return NULL;
    
    const char *boundary_start = strstr(content_type, "boundary=");
    if (!boundary_start) return NULL;
    
    boundary_start += strlen("boundary=");
    
    // Skip quotes if present
    if (*boundary_start == '"') {
        boundary_start++;
    }
    
    // Find end of boundary
    const char *boundary_end = boundary_start;
    while (*boundary_end && *boundary_end != '"' && 
           *boundary_end != ';' && *boundary_end != ' ') {
        boundary_end++;
    }
    
    size_t boundary_len = boundary_end - boundary_start;
    if (boundary_len == 0 || boundary_len >= MAX_BOUNDARY_SIZE) {
        return NULL;
    }
    
    char *boundary = malloc(boundary_len + 1);
    strncpy(boundary, boundary_start, boundary_len);
    boundary[boundary_len] = '\0';
    
    printf("[DEBUG] Extracted boundary: '%s'\n", boundary);
    return boundary;
}

// Parse Content-Disposition header for field information
void parse_content_disposition(const char *header, char *field_name, char *filename) {
    field_name[0] = '\0';
    filename[0] = '\0';
    
    // Extract name parameter
    const char *name_start = strstr(header, "name=\"");
    if (name_start) {
        name_start += strlen("name=\"");
        const char *name_end = strchr(name_start, '"');
        if (name_end) {
            size_t name_len = name_end - name_start;
            if (name_len < MAX_FIELD_NAME_SIZE) {
                strncpy(field_name, name_start, name_len);
                field_name[name_len] = '\0';
            }
        }
    }
    
    // Extract filename parameter (for file uploads)
    const char *filename_start = strstr(header, "filename=\"");
    if (filename_start) {
        filename_start += strlen("filename=\"");
        const char *filename_end = strchr(filename_start, '"');
        if (filename_end) {
            size_t filename_len = filename_end - filename_start;
            if (filename_len < MAX_FILENAME_SIZE) {
                strncpy(filename, filename_start, filename_len);
                filename[filename_len] = '\0';
            }
        }
    }
}

int parse_multipart_form(FormData *form, const char *body, const char *boundary) {
    if (!form || !body || !boundary) {
        return 0;
    }
    
    form_data_init(form);
    strncpy(form->boundary, boundary, sizeof(form->boundary) - 1);
    
    printf("[DEBUG] Parsing multipart form data with boundary: %s\n", boundary);
    
    // Create boundary markers
    char start_boundary[MAX_BOUNDARY_SIZE + 10];
    char end_boundary[MAX_BOUNDARY_SIZE + 10];
    snprintf(start_boundary, sizeof(start_boundary), "--%s", boundary);
    snprintf(end_boundary, sizeof(end_boundary), "--%s--", boundary);
    
    const char *current = body;
    
    // Skip to first boundary
    const char *boundary_pos = strstr(current, start_boundary);
    if (!boundary_pos) {
        form->error_message = strdup("No multipart boundary found");
        return 0;
    }
    
    current = boundary_pos + strlen(start_boundary);
    
    while (form->field_count < MAX_FORM_FIELDS) {
        // Skip CRLF after boundary
        while (*current == '\r' || *current == '\n') current++;
        
        // Check for end boundary
        if (strncmp(current, end_boundary, strlen(end_boundary)) == 0) {
            break;
        }
        
        // Find end of headers (double CRLF)
        const char *headers_end = strstr(current, "\r\n\r\n");
        if (!headers_end) {
            headers_end = strstr(current, "\n\n");
            if (!headers_end) break;
            headers_end += 2;
        } else {
            headers_end += 4;
        }
        
        // Parse headers
        char headers[1024];
        size_t headers_len = headers_end - current;
        if (headers_len >= sizeof(headers)) headers_len = sizeof(headers) - 1;
        strncpy(headers, current, headers_len);
        headers[headers_len] = '\0';
        
        printf("[DEBUG] Multipart headers: %.200s\n", headers);
        
        FormField *field = &form->fields[form->field_count];
        
        // Parse Content-Disposition
        const char *content_disp = strstr(headers, "Content-Disposition:");
        if (content_disp) {
            parse_content_disposition(content_disp, field->name, field->filename);
        }
        
        // Parse Content-Type (for files)
        const char *content_type = strstr(headers, "Content-Type:");
        if (content_type) {
            const char *type_start = content_type + strlen("Content-Type:");
            while (*type_start == ' ') type_start++;
            const char *type_end = strchr(type_start, '\r');
            if (!type_end) type_end = strchr(type_start, '\n');
            if (type_end) {
                size_t type_len = type_end - type_start;
                if (type_len < MAX_CONTENT_TYPE_SIZE) {
                    strncpy(field->content_type, type_start, type_len);
                    field->content_type[type_len] = '\0';
                }
            }
        }
        
        // Find field data (between headers and next boundary)
        current = headers_end;
        const char *next_boundary = strstr(current, start_boundary);
        if (!next_boundary) {
            next_boundary = strstr(current, end_boundary);
        }
        
        if (!next_boundary) break;
        
        // Calculate data length (excluding trailing CRLF)
        size_t data_len = next_boundary - current;
        while (data_len > 0 && (current[data_len - 1] == '\r' || current[data_len - 1] == '\n')) {
            data_len--;
        }
        
        // Determine field type and store data
        if (field->filename[0] != '\0') {
            // File field
            field->type = FORM_FIELD_FILE;
            field->file_data = current;
            field->file_size = data_len;
            
            printf("[DEBUG] Parsed file field: name='%s', filename='%s', size=%zu, type='%s'\n",
                   field->name, field->filename, field->file_size, field->content_type);
        } else {
            // Text field
            field->type = FORM_FIELD_TEXT;
            if (data_len < MAX_FIELD_VALUE_SIZE) {
                strncpy(field->value, current, data_len);
                field->value[data_len] = '\0';
            } else {
                strncpy(field->value, current, MAX_FIELD_VALUE_SIZE - 1);
                field->value[MAX_FIELD_VALUE_SIZE - 1] = '\0';
            }
            
            printf("[DEBUG] Parsed text field: name='%s', value='%.50s'\n",
                   field->name, field->value);
        }
        
        form->field_count++;
        current = next_boundary;
        
        // Skip boundary for next iteration
        if (strncmp(current, end_boundary, strlen(end_boundary)) != 0) {
            current += strlen(start_boundary);
        }
    }
    
    form->parsed = 1;
    printf("[DEBUG] Parsed %d multipart form fields\n", form->field_count);
    return 1;
}

// ============================================================================
// FORM DATA ACCESS FUNCTIONS
// ============================================================================

FormField* form_data_get_field(FormData *form, const char *name) {
    if (!form || !name) return NULL;
    
    for (int i = 0; i < form->field_count; i++) {
        if (strcmp(form->fields[i].name, name) == 0) {
            return &form->fields[i];
        }
    }
    return NULL;
}

const char* form_data_get_value(FormData *form, const char *name) {
    FormField *field = form_data_get_field(form, name);
    if (field && field->type == FORM_FIELD_TEXT) {
        return field->value;
    }
    return NULL;
}

FormField* form_data_get_file(FormData *form, const char *name) {
    FormField *field = form_data_get_field(form, name);
    if (field && field->type == FORM_FIELD_FILE) {
        return field;
    }
    return NULL;
}

int form_data_has_field(FormData *form, const char *name) {
    return form_data_get_field(form, name) != NULL;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void form_url_decode(char *dst, const char *src) {
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

void form_data_cleanup(FormData *form) {
    if (!form) return;
    
    if (form->error_message) {
        free(form->error_message);
        form->error_message = NULL;
    }
    
    form->field_count = 0;
    form->parsed = 0;
}

const char* form_field_type_name(FormFieldType type) {
    switch (type) {
        case FORM_FIELD_TEXT: return "text";
        case FORM_FIELD_FILE: return "file";
        default: return "unknown";
    }
}
