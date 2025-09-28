#include "app.h"
#include "form.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test handler for URL-encoded form submission
void form_submit_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next;
    (void)client_fd;
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    Response *res = (Response *)ctx->user_context;
    
    printf("[DEBUG] form_submit_handler: Processing form submission\n");
    
    // Check content type
    const char *content_type = req->get_header(req, "Content-Type");
    printf("[DEBUG] Content-Type: %s\n", content_type ? content_type : "none");
    
    if (!req->get_form(req)) {
        printf("[DEBUG] Failed to parse form data\n");
        response_status(res, 400);
        response_set_header(res, "Content-Type", "application/json");
        response_send(res, "{\"error\":\"Invalid form data\",\"message\":\"Could not parse form data\"}");
        return;
    }
    
    // Extract form values
    const char *name = req->get_form_value(req, "name");
    const char *email = req->get_form_value(req, "email");
    const char *message = req->get_form_value(req, "message");
    const char *newsletter = req->get_form_value(req, "newsletter");
    
    printf("[DEBUG] Form values - name: %s, email: %s, message: %.50s, newsletter: %s\n",
           name ? name : "null",
           email ? email : "null",
           message ? message : "null", 
           newsletter ? newsletter : "null");
    
    char response_body[2048];
    snprintf(response_body, sizeof(response_body),
        "{"
        "\"message\": \"Form submitted successfully\","
        "\"data\": {"
        "  \"name\": \"%s\","
        "  \"email\": \"%s\","
        "  \"message\": \"%.200s\","
        "  \"newsletter\": %s"
        "}"
        "}",
        name ? name : "",
        email ? email : "",
        message ? message : "",
        newsletter ? "true" : "false");
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response_body);
}

// Test handler for multipart form with file upload
void file_upload_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next;
    (void)client_fd;
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    Response *res = (Response *)ctx->user_context;
    
    printf("[DEBUG] file_upload_handler: Processing file upload\n");
    
    if (!req->get_form(req)) {
        printf("[DEBUG] Failed to parse multipart form data\n");
        response_status(res, 400);
        response_set_header(res, "Content-Type", "application/json");
        response_send(res, "{\"error\":\"Invalid multipart data\",\"message\":\"Could not parse multipart form data\"}");
        return;
    }
    
    // Get text fields
    const char *title = req->get_form_value(req, "title");
    const char *description = req->get_form_value(req, "description");
    
    // Get file field
    FormField *file_field = req->get_form_file(req, "document");
    
    printf("[DEBUG] Multipart values - title: %s, description: %.50s\n",
           title ? title : "null",
           description ? description : "null");
    
    if (file_field) {
        printf("[DEBUG] File upload - filename: %s, size: %zu, type: %s\n",
               file_field->filename, file_field->file_size, file_field->content_type);
        
        // Print file content for verification
        if (file_field->file_data && file_field->file_size > 0) {
            printf("[DEBUG] File content (%zu bytes):\n", file_field->file_size);
            printf("========== FILE CONTENT START ==========\n");
            
            // Print the file content (safely handle non-null-terminated data)
            for (size_t i = 0; i < file_field->file_size; i++) {
                char c = file_field->file_data[i];
                if (c >= 32 && c <= 126) {
                    // Printable ASCII character
                    putchar(c);
                } else if (c == '\n') {
                    // Newline
                    putchar('\n');
                } else if (c == '\r') {
                    // Carriage return (show as \r)
                    printf("\\r");
                } else if (c == '\t') {
                    // Tab
                    putchar('\t');
                } else {
                    // Non-printable character - show as hex
                    printf("\\x%02x", (unsigned char)c);
                }
            }
            printf("\n========== FILE CONTENT END ==========\n");
            
            // Also show hex dump of first 100 bytes for binary verification
            printf("[DEBUG] Hex dump (first %zu bytes):\n", 
                   file_field->file_size > 100 ? 100 : file_field->file_size);
            for (size_t i = 0; i < file_field->file_size && i < 100; i++) {
                if (i % 16 == 0) printf("%04zx: ", i);
                printf("%02x ", (unsigned char)file_field->file_data[i]);
                if (i % 16 == 15) printf("\n");
            }
            if (file_field->file_size % 16 != 0) printf("\n");
        } else {
            printf("[DEBUG] File field exists but no data or zero size\n");
        }
    } else {
        printf("[DEBUG] No file uploaded\n");
    }
    
    char response_body[2048];
    snprintf(response_body, sizeof(response_body),
        "{"
        "\"message\": \"File upload processed\","
        "\"data\": {"
        "  \"title\": \"%s\","
        "  \"description\": \"%.200s\","
        "  \"file\": {"
        "    \"uploaded\": %s,"
        "    \"filename\": \"%s\","
        "    \"size\": %zu,"
        "    \"content_type\": \"%s\""
        "  }"
        "}"
        "}",
        title ? title : "",
        description ? description : "",
        file_field ? "true" : "false",
        file_field ? file_field->filename : "",
        file_field ? file_field->file_size : 0,
        file_field ? file_field->content_type : "");
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response_body);
}

// Test handler to show all form fields
void form_debug_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next;
    (void)client_fd;
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    Response *res = (Response *)ctx->user_context;
    
    printf("[DEBUG] form_debug_handler: Debugging form fields\n");
    
    FormData *form = req->get_form(req);
    if (!form) {
        printf("[DEBUG] No form data available\n");
        response_status(res, 400);
        response_set_header(res, "Content-Type", "application/json");
        response_send(res, "{\"error\":\"No form data\",\"message\":\"Request does not contain form data\"}");
        return;
    }
    
    printf("[DEBUG] Found %d form fields\n", form->field_count);
    
    char response_body[4096];
    strcpy(response_body, "{\"message\":\"Form debug info\",\"fields\":[");
    
    for (int i = 0; i < form->field_count; i++) {
        FormField *field = &form->fields[i];
        char field_json[512];
        
        if (field->type == FORM_FIELD_FILE) {
            snprintf(field_json, sizeof(field_json),
                "%s{\"name\":\"%s\",\"type\":\"file\",\"filename\":\"%s\",\"size\":%zu,\"content_type\":\"%s\"}",
                i > 0 ? "," : "",
                field->name,
                field->filename,
                field->file_size,
                field->content_type);
        } else {
            snprintf(field_json, sizeof(field_json),
                "%s{\"name\":\"%s\",\"type\":\"text\",\"value\":\"%.100s\"}",
                i > 0 ? "," : "",
                field->name,
                field->value);
        }
        
        strcat(response_body, field_json);
    }
    
    strcat(response_body, "]}");
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response_body);
}

// Test handler for mixed content type detection
void content_type_test_handler(int client_fd, void (*next)(void *), void *context) {
    (void)next;
    (void)client_fd;
    
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    Response *res = (Response *)ctx->user_context;
    
    const char *content_type = req->get_header(req, "Content-Type");
    
    char response_body[1024];
    snprintf(response_body, sizeof(response_body),
        "{"
        "\"content_type\": \"%s\","
        "\"is_json\": %s,"
        "\"is_form_data\": %s,"
        "\"is_multipart\": %s"
        "}",
        content_type ? content_type : "none",
        request_is_json(req) ? "true" : "false",
        request_is_form_data(req) ? "true" : "false",
        request_is_multipart_form(req) ? "true" : "false");
    
    response_set_header(res, "Content-Type", "application/json");
    response_send(res, response_body);
}

int main() {
    printf("=== Testing Form Data Handling ===\n\n");
    
    App app = create_app();
    
    printf("Setting up Form Data endpoints...\n\n");
    
    // Form data endpoints
    printf("  POST /contact - URL-encoded form submission\n");
    app.post(&app, "/contact", form_submit_handler);
    
    printf("  POST /upload - Multipart form with file upload\n");
    app.post(&app, "/upload", file_upload_handler);
    
    printf("  POST /debug - Show all form fields\n");
    app.post(&app, "/debug", form_debug_handler);
    
    printf("  POST /detect - Content type detection test\n");
    app.post(&app, "/detect", content_type_test_handler);
    
    printf("\n=== Form Data Features ===\n");
    printf("✓ URL-encoded form parsing (application/x-www-form-urlencoded)\n");
    printf("✓ Multipart form parsing (multipart/form-data)\n");
    printf("✓ File upload support with metadata\n");
    printf("✓ Form field access by name\n");
    printf("✓ Content-Type detection and validation\n");
    printf("✓ Memory-safe form parsing and cleanup\n");
    printf("✓ Support for text fields and file fields\n");
    
    printf("\n=== Testing Instructions ===\n");
    printf("Starting server on port 3000...\n\n");
    
    printf("Test these form endpoints:\n\n");
    
    printf("1. URL-encoded form submission:\n");
    printf("   curl -X POST http://localhost:3000/contact \\\n");
    printf("     -H 'Content-Type: application/x-www-form-urlencoded' \\\n");
    printf("     -d 'name=John+Doe&email=john@example.com&message=Hello+World&newsletter=on'\n\n");
    
    printf("2. Simple form without newsletter:\n");
    printf("   curl -X POST http://localhost:3000/contact \\\n");
    printf("     -H 'Content-Type: application/x-www-form-urlencoded' \\\n");
    printf("     -d 'name=Jane&email=jane@test.com&message=Testing+form+submission'\n\n");
    
    printf("3. Multipart form with file (create a test file first):\n");
    printf("   echo 'Test file content' > test.txt\n");
    printf("   curl -X POST http://localhost:3000/upload \\\n");
    printf("     -F 'title=My Document' \\\n");
    printf("     -F 'description=A test document upload' \\\n");
    printf("     -F 'document=@test.txt'\n\n");
    
    printf("4. Form debug (URL-encoded):\n");
    printf("   curl -X POST http://localhost:3000/debug \\\n");
    printf("     -H 'Content-Type: application/x-www-form-urlencoded' \\\n");
    printf("     -d 'field1=value1&field2=value2&field3=value+with+spaces'\n\n");
    
    printf("5. Form debug (multipart):\n");
    printf("   echo 'Sample content' > sample.txt\n");
    printf("   curl -X POST http://localhost:3000/debug \\\n");
    printf("     -F 'text_field=Hello' \\\n");
    printf("     -F 'another_field=World' \\\n");
    printf("     -F 'file_field=@sample.txt'\n\n");
    
    printf("6. Content type detection:\n");
    printf("   curl -X POST http://localhost:3000/detect \\\n");
    printf("     -H 'Content-Type: application/x-www-form-urlencoded' \\\n");
    printf("     -d 'test=data'\n\n");
    
    printf("7. Content type detection (multipart):\n");
    printf("   curl -X POST http://localhost:3000/detect \\\n");
    printf("     -F 'test=data'\n\n");
    
    printf("8. Content type detection (JSON):\n");
    printf("   curl -X POST http://localhost:3000/detect \\\n");
    printf("     -H 'Content-Type: application/json' \\\n");
    printf("     -d '{\"test\":\"data\"}'\n\n");
    
    app_listen(&app, 3000);
    
    return 0;
}
