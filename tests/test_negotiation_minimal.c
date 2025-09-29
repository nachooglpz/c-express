#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Simple HTTP content negotiation test without full framework
// This demonstrates content negotiation concepts standalone

typedef struct {
    char media_type[64];
    float quality;
    int specificity;
} AcceptType;

typedef struct {
    AcceptType types[16];
    int count;
} ContentNegotiation;

// Parse Accept header and extract MIME types with quality values
ContentNegotiation parse_accept_header(const char *accept_header) {
    ContentNegotiation negotiation = {0};
    
    if (!accept_header) {
        // Default to HTML if no Accept header
        strcpy(negotiation.types[0].media_type, "text/html");
        negotiation.types[0].quality = 1.0f;
        negotiation.types[0].specificity = 2;
        negotiation.count = 1;
        return negotiation;
    }
    
    printf("[DEBUG] Parsing Accept header: %s\n", accept_header);
    
    char *header_copy = strdup(accept_header);
    char *token = strtok(header_copy, ",");
    
    while (token && negotiation.count < 16) {
        // Trim whitespace
        while (*token == ' ') token++;
        
        AcceptType *type = &negotiation.types[negotiation.count];
        type->quality = 1.0f; // Default quality
        type->specificity = 0;
        
        // Look for quality parameter
        char *semicolon = strchr(token, ';');
        if (semicolon) {
            *semicolon = '\0';
            char *q_param = strstr(semicolon + 1, "q=");
            if (q_param) {
                type->quality = atof(q_param + 2);
            }
        }
        
        // Copy media type
        strncpy(type->media_type, token, sizeof(type->media_type) - 1);
        type->media_type[sizeof(type->media_type) - 1] = '\0';
        
        // Calculate specificity
        if (strstr(type->media_type, "*/*")) {
            type->specificity = 0;
        } else if (strstr(type->media_type, "/*")) {
            type->specificity = 1;
        } else {
            type->specificity = 2;
        }
        
        printf("[DEBUG] Parsed: %s (q=%.2f, specificity=%d)\n", 
               type->media_type, type->quality, type->specificity);
        
        negotiation.count++;
        token = strtok(NULL, ",");
    }
    
    free(header_copy);
    
    // Sort by quality (descending) then by specificity (descending)
    for (int i = 0; i < negotiation.count - 1; i++) {
        for (int j = i + 1; j < negotiation.count; j++) {
            AcceptType *a = &negotiation.types[i];
            AcceptType *b = &negotiation.types[j];
            
            if (b->quality > a->quality || 
                (b->quality == a->quality && b->specificity > a->specificity)) {
                AcceptType temp = *a;
                *a = *b;
                *b = temp;
            }
        }
    }
    
    return negotiation;
}

// Negotiate best content type from available options
const char* negotiate_content_type(const char *accept_header, const char **available_types, int count) {
    ContentNegotiation negotiation = parse_accept_header(accept_header);
    
    printf("[DEBUG] Negotiating content type from %d available options\n", count);
    
    // Try each accepted type in preference order
    for (int i = 0; i < negotiation.count; i++) {
        AcceptType *accepted = &negotiation.types[i];
        
        // Check exact matches first
        for (int j = 0; j < count; j++) {
            if (strcmp(accepted->media_type, available_types[j]) == 0) {
                printf("[DEBUG] Exact match: %s\n", available_types[j]);
                return available_types[j];
            }
        }
        
        // Check wildcard matches
        if (strstr(accepted->media_type, "*/*")) {
            printf("[DEBUG] Wildcard match: %s\n", available_types[0]);
            return available_types[0]; // Return first available
        }
        
        if (strstr(accepted->media_type, "/*")) {
            char main_type[32];
            strncpy(main_type, accepted->media_type, strchr(accepted->media_type, '/') - accepted->media_type);
            main_type[strchr(accepted->media_type, '/') - accepted->media_type] = '\0';
            
            for (int j = 0; j < count; j++) {
                if (strncmp(available_types[j], main_type, strlen(main_type)) == 0) {
                    printf("[DEBUG] Type wildcard match: %s\n", available_types[j]);
                    return available_types[j];
                }
            }
        }
    }
    
    printf("[DEBUG] No acceptable content type found\n");
    return NULL; // No acceptable type found
}

// Generate sample data in different formats
void generate_user_data(const char *content_type, char *output, size_t max_size) {
    if (strcmp(content_type, "application/json") == 0) {
        snprintf(output, max_size,
            "{\n"
            "  \"users\": [\n"
            "    {\"id\": 1, \"name\": \"Alice\", \"email\": \"alice@example.com\"},\n"
            "    {\"id\": 2, \"name\": \"Bob\", \"email\": \"bob@example.com\"},\n"
            "    {\"id\": 3, \"name\": \"Charlie\", \"email\": \"charlie@example.com\"}\n"
            "  ],\n"
            "  \"total\": 3\n"
            "}");
    } else if (strcmp(content_type, "text/html") == 0) {
        snprintf(output, max_size,
            "<!DOCTYPE html>\n"
            "<html><head><title>Users</title></head>\n"
            "<body>\n"
            "  <h1>Content Negotiation Demo - Users List</h1>\n"
            "  <table border='1'>\n"
            "    <tr><th>ID</th><th>Name</th><th>Email</th></tr>\n"
            "    <tr><td>1</td><td>Alice</td><td>alice@example.com</td></tr>\n"
            "    <tr><td>2</td><td>Bob</td><td>bob@example.com</td></tr>\n"
            "    <tr><td>3</td><td>Charlie</td><td>charlie@example.com</td></tr>\n"
            "  </table>\n"
            "  <p>Total: 3 users</p>\n"
            "</body></html>");
    } else if (strcmp(content_type, "application/xml") == 0) {
        snprintf(output, max_size,
            "<?xml version='1.0' encoding='UTF-8'?>\n"
            "<users total='3'>\n"
            "  <user id='1'>\n"
            "    <name>Alice</name>\n"
            "    <email>alice@example.com</email>\n"
            "  </user>\n"
            "  <user id='2'>\n"
            "    <name>Bob</name>\n"
            "    <email>bob@example.com</email>\n"
            "  </user>\n"
            "  <user id='3'>\n"
            "    <name>Charlie</name>\n"
            "    <email>charlie@example.com</email>\n"
            "  </user>\n"
            "</users>");
    } else if (strcmp(content_type, "text/csv") == 0) {
        snprintf(output, max_size,
            "ID,Name,Email\n"
            "1,Alice,alice@example.com\n"
            "2,Bob,bob@example.com\n"
            "3,Charlie,charlie@example.com");
    } else {
        snprintf(output, max_size,
            "Users List:\n"
            "1. Alice (alice@example.com)\n"
            "2. Bob (bob@example.com)\n"
            "3. Charlie (charlie@example.com)\n"
            "Total: 3 users");
    }
}

// Simple HTTP response sender
void send_http_response(int client_fd, int status_code, const char *content_type, const char *body) {
    char response[4096];
    const char *status_text = (status_code == 200) ? "OK" : 
                             (status_code == 406) ? "Not Acceptable" : "Internal Server Error";
    
    snprintf(response, sizeof(response),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status_code, status_text, content_type, strlen(body), body);
    
    send(client_fd, response, strlen(response), 0);
}

// Handle content negotiation request
void handle_request(int client_fd, const char *accept_header) {
    printf("\n=== Content Negotiation Request ===\n");
    printf("Accept header: %s\n", accept_header ? accept_header : "(none)");
    
    // Available content types for this endpoint
    const char *available[] = {
        "application/json",
        "text/html", 
        "application/xml",
        "text/csv",
        "text/plain"
    };
    int available_count = sizeof(available) / sizeof(available[0]);
    
    // Negotiate best content type
    const char *chosen_type = negotiate_content_type(accept_header, available, available_count);
    
    if (!chosen_type) {
        // Send 406 Not Acceptable
        const char *error_body = 
            "{\n"
            "  \"error\": \"Not Acceptable\",\n"
            "  \"message\": \"No acceptable content type found\",\n"
            "  \"available\": [\"application/json\", \"text/html\", \"application/xml\", \"text/csv\", \"text/plain\"]\n"
            "}";
        send_http_response(client_fd, 406, "application/json", error_body);
        printf("Sent 406 Not Acceptable\n");
        return;
    }
    
    printf("Negotiated content type: %s\n", chosen_type);
    
    // Generate content in chosen format
    char content[2048];
    generate_user_data(chosen_type, content, sizeof(content));
    
    // Send response
    send_http_response(client_fd, 200, chosen_type, content);
    printf("Response sent with %s content\n", chosen_type);
}

int main() {
    printf("Content Negotiation Test Server\n");
    printf("==================================\n\n");
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        return 1;
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(3000);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return 1;
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        return 1;
    }
    
    printf("Server running on http://localhost:3000\n");
    printf("\nTry these curl commands:\n");
    printf("curl -H \"Accept: application/json\" http://localhost:3000/\n");
    printf("curl -H \"Accept: text/html\" http://localhost:3000/\n");
    printf("curl -H \"Accept: application/xml\" http://localhost:3000/\n");
    printf("curl -H \"Accept: text/csv\" http://localhost:3000/\n");
    printf("curl -H \"Accept: text/plain\" http://localhost:3000/\n");
    printf("curl -H \"Accept: application/pdf\" http://localhost:3000/  # Should get 406\n");
    printf("curl -H \"Accept: */*, application/json;q=0.8\" http://localhost:3000/\n");
    printf("curl -H \"Accept: text/*\" http://localhost:3000/\n");
    printf("\nPress Ctrl+C to stop...\n\n");
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }
        
        // Read HTTP request
        char buffer[4096] = {0};
        read(client_fd, buffer, sizeof(buffer) - 1);
        
        // Parse Accept header
        char *accept_header = NULL;
        char *accept_line = strstr(buffer, "Accept: ");
        if (accept_line) {
            accept_line += 8; // Skip "Accept: "
            char *end = strstr(accept_line, "\r\n");
            if (end) {
                size_t len = end - accept_line;
                accept_header = malloc(len + 1);
                strncpy(accept_header, accept_line, len);
                accept_header[len] = '\0';
            }
        }
        
        handle_request(client_fd, accept_header);
        
        if (accept_header) {
            free(accept_header);
        }
        
        close(client_fd);
    }
    
    close(server_fd);
    return 0;
}
