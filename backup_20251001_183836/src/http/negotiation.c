#include "negotiation.h"
#include "debug.h"
#include <stdio.h>
#include "debug.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Fix missing includes for strdup and other functions
#define _GNU_SOURCE

// Content format definitions
static const ContentFormat content_formats[] = {
    {CONTENT_JSON, "application/json", "json", "JSON"},
    {CONTENT_XML, "application/xml", "xml", "XML"},
    {CONTENT_HTML, "text/html", "html", "HTML"},
    {CONTENT_TEXT, "text/plain", "txt", "Plain Text"},
    {CONTENT_CSV, "text/csv", "csv", "CSV"},
    {CONTENT_YAML, "application/yaml", "yaml", "YAML"},
    {CONTENT_FORM_URLENCODED, "application/x-www-form-urlencoded", "", "Form URL Encoded"},
    {CONTENT_MULTIPART, "multipart/form-data", "", "Multipart Form Data"},
    {CONTENT_BINARY, "application/octet-stream", "bin", "Binary"},
    {CONTENT_UNKNOWN, "*/*", "", "Unknown"}
};

static const int content_formats_count = sizeof(content_formats) / sizeof(ContentFormat);

// Parse a single media type with quality value
static void parse_media_type(const char *media_type_str, AcceptType *accept_type) {
    char buffer[256];
    strncpy(buffer, media_type_str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    // Initialize defaults
    accept_type->quality = 1.0f;
    accept_type->wildcard_main = 0;
    accept_type->wildcard_sub = 0;
    
    // Find q-value if present
    char *q_start = strstr(buffer, ";q=");
    if (q_start) {
        accept_type->quality = atof(q_start + 3);
        *q_start = '\0';  // Terminate the media type part
    }
    
    // Trim whitespace
    char *start = buffer;
    while (isspace(*start)) start++;
    char *end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) *end-- = '\0';
    
    // Parse main type and subtype
    char *slash = strchr(start, '/');
    if (slash) {
        *slash = '\0';
        strncpy(accept_type->main_type, start, sizeof(accept_type->main_type) - 1);
        accept_type->main_type[sizeof(accept_type->main_type) - 1] = '\0';
        
        strncpy(accept_type->subtype, slash + 1, sizeof(accept_type->subtype) - 1);
        accept_type->subtype[sizeof(accept_type->subtype) - 1] = '\0';
        
        // Build full media type
        int result = snprintf(accept_type->media_type, sizeof(accept_type->media_type), 
                 "%s/%s", accept_type->main_type, accept_type->subtype);
        if (result >= (int)sizeof(accept_type->media_type)) {
            // Truncated, set to wildcard as fallback
            strcpy(accept_type->media_type, "*/*");
        }
    } else {
        // Invalid format, treat as wildcard
        strcpy(accept_type->main_type, "*");
        strcpy(accept_type->subtype, "*");
        strcpy(accept_type->media_type, "*/*");
    }
    
    // Check for wildcards
    if (strcmp(accept_type->main_type, "*") == 0) {
        accept_type->wildcard_main = 1;
    }
    if (strcmp(accept_type->subtype, "*") == 0) {
        accept_type->wildcard_sub = 1;
    }
    
    printf("[DEBUG] Parsed media type: %s (q=%.2f, main=%s, sub=%s)\n",
           accept_type->media_type, accept_type->quality, 
           accept_type->main_type, accept_type->subtype);
}

// Compare accept types for sorting (higher quality first)
static int compare_accept_types(const void *a, const void *b) {
    const AcceptType *type_a = (const AcceptType *)a;
    const AcceptType *type_b = (const AcceptType *)b;
    
    // Sort by quality (descending)
    if (type_a->quality > type_b->quality) return -1;
    if (type_a->quality < type_b->quality) return 1;
    
    // If equal quality, prefer more specific types
    if (type_a->wildcard_main && !type_b->wildcard_main) return 1;
    if (!type_a->wildcard_main && type_b->wildcard_main) return -1;
    if (type_a->wildcard_sub && !type_b->wildcard_sub) return 1;
    if (!type_a->wildcard_sub && type_b->wildcard_sub) return -1;
    
    return 0;
}

ContentNegotiation* parse_accept_header(Request *req) {
    ContentNegotiation *negotiation = malloc(sizeof(ContentNegotiation));
    if (!negotiation) return NULL;
    
    negotiation->accept_count = 0;
    negotiation->preferred_language[0] = '\0';
    negotiation->preferred_encoding[0] = '\0';
    negotiation->preferred_charset[0] = '\0';
    
    const char *accept_header = req->get_header(req, "Accept");
    if (!accept_header) {
        // Default to accepting everything
        parse_media_type("*/*", &negotiation->accept_types[0]);
        negotiation->accept_count = 1;
        printf("[DEBUG] No Accept header, defaulting to */*\n");
        return negotiation;
    }
    
    printf("[DEBUG] Parsing Accept header: %s\n", accept_header);
    
    // Parse comma-separated media types
    char buffer[1024];
    strncpy(buffer, accept_header, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    char *token = strtok(buffer, ",");
    while (token && negotiation->accept_count < MAX_ACCEPT_TYPES) {
        parse_media_type(token, &negotiation->accept_types[negotiation->accept_count]);
        negotiation->accept_count++;
        token = strtok(NULL, ",");
    }
    
    // Sort by quality and specificity
    qsort(negotiation->accept_types, negotiation->accept_count, 
          sizeof(AcceptType), compare_accept_types);
    
    // Parse other negotiation headers
    const char *accept_language = req->get_header(req, "Accept-Language");
    if (accept_language) {
        strncpy(negotiation->preferred_language, accept_language, 
                sizeof(negotiation->preferred_language) - 1);
    }
    
    const char *accept_encoding = req->get_header(req, "Accept-Encoding");
    if (accept_encoding) {
        strncpy(negotiation->preferred_encoding, accept_encoding, 
                sizeof(negotiation->preferred_encoding) - 1);
    }
    
    const char *accept_charset = req->get_header(req, "Accept-Charset");
    if (accept_charset) {
        strncpy(negotiation->preferred_charset, accept_charset, 
                sizeof(negotiation->preferred_charset) - 1);
    }
    
    return negotiation;
}

const char* negotiate_content_type(ContentNegotiation *negotiation, 
                                  const char **available_types, int count) {
    if (!negotiation || !available_types || count == 0) {
        return "text/plain";  // Default fallback
    }
    
    printf("[DEBUG] Negotiating content type from %d available types\n", count);
    
    // Try each accept type in order (already sorted by quality)
    for (int i = 0; i < negotiation->accept_count; i++) {
        AcceptType *accept = &negotiation->accept_types[i];
        
        printf("[DEBUG] Checking accept type: %s (q=%.2f)\n", 
               accept->media_type, accept->quality);
        
        // Check against available types
        for (int j = 0; j < count; j++) {
            const char *available = available_types[j];
            
            // Exact match
            if (strcmp(accept->media_type, available) == 0) {
                printf("[DEBUG] Exact match found: %s\n", available);
                return available;
            }
            
            // Wildcard matching
            if (accept->wildcard_main && accept->wildcard_sub) {
                // */* matches everything
                printf("[DEBUG] Wildcard match (*/*): %s\n", available);
                return available;
            }
            
            // Parse available type for wildcard matching
            char main_type[64], sub_type[64];
            if (sscanf(available, "%63[^/]/%63s", main_type, sub_type) == 2) {
                if (accept->wildcard_sub && 
                    strcmp(accept->main_type, main_type) == 0) {
                    // text/* matches text/html, text/plain, etc.
                    printf("[DEBUG] Subtype wildcard match (%s/*): %s\n", 
                           main_type, available);
                    return available;
                }
            }
        }
    }
    
    // No match found, return first available type or default
    if (count > 0) {
        printf("[DEBUG] No negotiated match, using first available: %s\n", 
               available_types[0]);
        return available_types[0];
    }
    
    printf("[DEBUG] No types available, using default: text/plain\n");
    return "text/plain";
}

ContentType get_content_type_enum(const char *mime_type) {
    if (!mime_type) return CONTENT_UNKNOWN;
    
    for (int i = 0; i < content_formats_count; i++) {
        if (strcmp(content_formats[i].mime_type, mime_type) == 0) {
            return content_formats[i].type;
        }
    }
    
    return CONTENT_UNKNOWN;
}

const char* get_mime_type_string(ContentType type) {
    if (type >= 0 && (size_t)type < (size_t)content_formats_count) {
        return content_formats[type].mime_type;
    }
    return "text/plain";
}

int accepts_content_type(ContentNegotiation *negotiation, const char *mime_type) {
    if (!negotiation || !mime_type) return 0;
    
    for (int i = 0; i < negotiation->accept_count; i++) {
        AcceptType *accept = &negotiation->accept_types[i];
        
        // Exact match
        if (strcmp(accept->media_type, mime_type) == 0) {
            return 1;
        }
        
        // Wildcard matching
        if (accept->wildcard_main && accept->wildcard_sub) {
            return 1;  // */* accepts everything
        }
        
        // Parse mime type for wildcard matching
        char main_type[64], sub_type[64];
        if (sscanf(mime_type, "%63[^/]/%63s", main_type, sub_type) == 2) {
            if (accept->wildcard_sub && 
                strcmp(accept->main_type, main_type) == 0) {
                return 1;  // text/* matches text/html, etc.
            }
        }
    }
    
    return 0;
}

float get_content_type_quality(ContentNegotiation *negotiation, const char *mime_type) {
    if (!negotiation || !mime_type) return 0.0f;
    
    for (int i = 0; i < negotiation->accept_count; i++) {
        AcceptType *accept = &negotiation->accept_types[i];
        
        if (strcmp(accept->media_type, mime_type) == 0) {
            return accept->quality;
        }
        
        // Check wildcard matches
        if (accept->wildcard_main && accept->wildcard_sub) {
            return accept->quality;
        }
        
        char main_type[64], sub_type[64];
        if (sscanf(mime_type, "%63[^/]/%63s", main_type, sub_type) == 2) {
            if (accept->wildcard_sub && 
                strcmp(accept->main_type, main_type) == 0) {
                return accept->quality;
            }
        }
    }
    
    return 0.0f;
}

// Helper functions for common content types
int accepts_json(Request *req) {
    ContentNegotiation *negotiation = parse_accept_header(req);
    int result = accepts_content_type(negotiation, "application/json");
    free_content_negotiation(negotiation);
    return result;
}

int accepts_html(Request *req) {
    ContentNegotiation *negotiation = parse_accept_header(req);
    int result = accepts_content_type(negotiation, "text/html");
    free_content_negotiation(negotiation);
    return result;
}

int accepts_xml(Request *req) {
    ContentNegotiation *negotiation = parse_accept_header(req);
    int result = accepts_content_type(negotiation, "application/xml");
    free_content_negotiation(negotiation);
    return result;
}

int accepts_text(Request *req) {
    ContentNegotiation *negotiation = parse_accept_header(req);
    int result = accepts_content_type(negotiation, "text/plain");
    free_content_negotiation(negotiation);
    return result;
}

// Enhanced response method with content negotiation
void response_negotiate_and_send(Response *res, Request *req, 
                                const char *json_data, const char *html_data, 
                                const char *text_data) {
    ContentNegotiation *negotiation = parse_accept_header(req);
    
    // Define available content types
    const char *available_types[] = {
        "application/json",
        "text/html", 
        "text/plain"
    };
    int available_count = 3;
    
    // Negotiate the best content type
    const char *best_type = negotiate_content_type(negotiation, available_types, available_count);
    
    printf("[DEBUG] Negotiated content type: %s\n", best_type);
    
    // Send appropriate content
    res->set_header(res, "Content-Type", best_type);
    
    if (strcmp(best_type, "application/json") == 0 && json_data) {
        res->send(res, json_data);
    } else if (strcmp(best_type, "text/html") == 0 && html_data) {
        res->send(res, html_data);
    } else if (strcmp(best_type, "text/plain") == 0 && text_data) {
        res->send(res, text_data);
    } else {
        // Fallback to text
        res->set_header(res, "Content-Type", "text/plain");
        res->send(res, text_data ? text_data : "No content available");
    }
    
    free_content_negotiation(negotiation);
}

char* format_response_data(ContentType type, const char *data) {
    if (!data) return NULL;
    
    size_t data_len = strlen(data);
    char *formatted = NULL;
    
    switch (type) {
        case CONTENT_JSON:
            // Assume data is already JSON formatted
            formatted = malloc(data_len + 1);
            if (formatted) strcpy(formatted, data);
            break;
            
        case CONTENT_HTML:
            // Wrap in basic HTML
            formatted = malloc(data_len + 200);
            if (formatted) {
                snprintf(formatted, data_len + 200, 
                        "<!DOCTYPE html><html><head><title>Response</title></head>"
                        "<body><pre>%s</pre></body></html>", data);
            }
            break;
            
        case CONTENT_TEXT:
        default:
            // Plain text, return as-is
            formatted = malloc(data_len + 1);
            if (formatted) strcpy(formatted, data);
            break;
    }
    
    return formatted;
}

void free_content_negotiation(ContentNegotiation *negotiation) {
    if (negotiation) {
        free(negotiation);
    }
}

void print_content_negotiation(ContentNegotiation *negotiation) {
    if (!negotiation) {
        printf("[DEBUG] Content negotiation: NULL\n");
        return;
    }
    
    printf("[DEBUG] Content Negotiation Summary:\n");
    printf("[DEBUG]   Accept types (%d):\n", negotiation->accept_count);
    
    for (int i = 0; i < negotiation->accept_count; i++) {
        AcceptType *accept = &negotiation->accept_types[i];
        printf("[DEBUG]     %d. %s (q=%.2f) %s%s\n", 
               i + 1, accept->media_type, accept->quality,
               accept->wildcard_main ? "[main wildcard] " : "",
               accept->wildcard_sub ? "[sub wildcard]" : "");
    }
    
    if (negotiation->preferred_language[0]) {
        printf("[DEBUG]   Preferred language: %s\n", negotiation->preferred_language);
    }
    if (negotiation->preferred_encoding[0]) {
        printf("[DEBUG]   Preferred encoding: %s\n", negotiation->preferred_encoding);
    }
    if (negotiation->preferred_charset[0]) {
        printf("[DEBUG]   Preferred charset: %s\n", negotiation->preferred_charset);
    }
}

// High-level API - simple content negotiation for common use cases
const char* get_preferred_content_type(Request *req, const char **available_types, int count) {
    if (!req || !available_types || count == 0) {
        return "application/json";  // Default fallback
    }
    
    // Parse Accept header using negotiation module
    ContentNegotiation *negotiation = parse_accept_header(req);
    if (!negotiation) {
        return "application/json";  // fallback
    }
    
    // Negotiate best content type
    const char *chosen_type = negotiate_content_type(negotiation, available_types, count);
    
    // Clean up
    free(negotiation);
    
    return chosen_type ? chosen_type : "application/json";
}

// Convenience function with common web content types
const char* get_preferred_web_content_type(Request *req) {
    // Common web content types in priority order
    static const char *common_types[] = {
        "application/json",
        "text/html",
        "application/xml", 
        "text/plain",
        "text/csv"
    };
    static const int common_count = sizeof(common_types) / sizeof(common_types[0]);
    
    return get_preferred_content_type(req, common_types, common_count);
}

const char* content_type_to_string(ContentType type) {
    if (type >= 0 && (size_t)type < (size_t)content_formats_count) {
        return content_formats[type].description;
    }
    return "Unknown";
}
