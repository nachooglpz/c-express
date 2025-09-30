#include "../src/core/app.h"
#include "../src/http/streaming.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Test handler for small uploads (should use legacy mode)
void small_upload_handler(int client_fd, void (*next)(void *), void *context) {
    (void)client_fd;
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    Response *res = (Response *)ctx->user_context;
    
    printf("\n[TEST] === Small Upload Test ===\n");
    
    char response_body[1024];
    
    if (req->is_body_streamed && req->is_body_streamed(req)) {
        printf("[TEST] ✅ Streaming mode detected\n");
        StreamContext *stream = req->get_stream(req);
        if (stream) {
            printf("[TEST] Mode: %s, Length: %zu, Chunked: %s\n",
                   (stream->mode == BODY_MODE_MEMORY) ? "MEMORY" : "DISK",
                   stream->content_length,
                   stream->is_chunked ? "YES" : "NO");
            
            snprintf(response_body, sizeof(response_body),
                "{\"test\":\"small\",\"mode\":\"streaming\",\"stream_mode\":\"%s\",\"size\":%zu}",
                (stream->mode == BODY_MODE_MEMORY) ? "memory" : "disk",
                stream->content_length);
        } else {
            snprintf(response_body, sizeof(response_body),
                "{\"test\":\"small\",\"error\":\"no_stream_context\"}");
        }
    } else {
        printf("[TEST] ✅ Legacy mode (expected for small uploads)\n");
        printf("[TEST] Body: %.50s\n", req->body);
        
        snprintf(response_body, sizeof(response_body),
            "{\"test\":\"small\",\"mode\":\"legacy\",\"size\":%zu,\"preview\":\"%.50s\"}",
            strlen(req->body), req->body);
    }
    
    res->set_header(res, "Content-Type", "application/json");
    res->send(res, response_body);
    next(ctx);
}

// Test handler for large uploads (should use streaming mode)
void large_upload_handler(int client_fd, void (*next)(void *), void *context) {
    (void)client_fd;
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    Response *res = (Response *)ctx->user_context;
    
    printf("\n[TEST] === Large Upload Test ===\n");
    
    char response_body[1024];
    
    if (req->is_body_streamed && req->is_body_streamed(req)) {
        printf("[TEST] ✅ Streaming mode (expected for large uploads)\n");
        StreamContext *stream = req->get_stream(req);
        if (stream) {
            printf("[TEST] Mode: %s, Length: %zu, Temp file: %s\n",
                   (stream->mode == BODY_MODE_STREAM) ? "DISK" : "MEMORY",
                   stream->content_length,
                   stream->temp_filename[0] ? stream->temp_filename : "none");
            
            snprintf(response_body, sizeof(response_body),
                "{\"test\":\"large\",\"mode\":\"streaming\",\"stream_mode\":\"%s\",\"size\":%zu,\"temp_file\":\"%s\"}",
                (stream->mode == BODY_MODE_STREAM) ? "disk" : "memory",
                stream->content_length,
                stream->temp_filename[0] ? stream->temp_filename : "none");
        } else {
            snprintf(response_body, sizeof(response_body),
                "{\"test\":\"large\",\"error\":\"no_stream_context\"}");
        }
    } else {
        printf("[TEST] ⚠️ Legacy mode (unexpected for large uploads)\n");
        snprintf(response_body, sizeof(response_body),
            "{\"test\":\"large\",\"mode\":\"legacy\",\"warning\":\"large_upload_in_legacy_mode\"}");
    }
    
    res->set_header(res, "Content-Type", "application/json");
    res->send(res, response_body);
    next(ctx);
}

// Test handler for chunked uploads
void chunked_upload_handler(int client_fd, void (*next)(void *), void *context) {
    (void)client_fd;
    NextContext *ctx = (NextContext *)context;
    Request *req = ctx->req;
    Response *res = (Response *)ctx->user_context;
    
    printf("\n[TEST] === Chunked Upload Test ===\n");
    
    char response_body[1024];
    
    if (req->is_body_streamed && req->is_body_streamed(req)) {
        printf("[TEST] ✅ Streaming mode (expected for chunked)\n");
        StreamContext *stream = req->get_stream(req);
        if (stream) {
            printf("[TEST] Is chunked: %s, Bytes read: %zu\n",
                   stream->is_chunked ? "YES" : "NO",
                   stream->bytes_read);
            
            snprintf(response_body, sizeof(response_body),
                "{\"test\":\"chunked\",\"mode\":\"streaming\",\"is_chunked\":%s,\"bytes_read\":%zu}",
                stream->is_chunked ? "true" : "false",
                stream->bytes_read);
        } else {
            snprintf(response_body, sizeof(response_body),
                "{\"test\":\"chunked\",\"error\":\"no_stream_context\"}");
        }
    } else {
        printf("[TEST] ⚠️ Legacy mode (unexpected for chunked)\n");
        snprintf(response_body, sizeof(response_body),
            "{\"test\":\"chunked\",\"mode\":\"legacy\",\"warning\":\"chunked_in_legacy_mode\"}");
    }
    
    res->set_header(res, "Content-Type", "application/json");
    res->send(res, response_body);
    next(ctx);
}

// Info handler
void info_handler(int client_fd, void (*next)(void *), void *context) {
    (void)client_fd;
    NextContext *ctx = (NextContext *)context;
    Response *res = (Response *)ctx->user_context;
    
    const char* info = 
        "{"
        "\"server\":\"C-Express Streaming Test Suite\","
        "\"endpoints\":["
        "\"/info - This endpoint\","
        "\"/test/small - Small upload test\","
        "\"/test/large - Large upload test\","
        "\"/test/chunked - Chunked upload test\""
        "],"
        "\"tests\":["
        "\"curl http://localhost:8080/info\","
        "\"curl -X POST http://localhost:8080/test/small -d 'small data'\","
        "\"curl -X POST http://localhost:8080/test/large -d \\\"$(head -c 20000 /dev/zero | tr '\\\\0' 'A')\\\"\","
        "\"curl -X POST http://localhost:8080/test/chunked -H 'Transfer-Encoding: chunked' -d 'chunked data'\""
        "]"
        "}";
    
    res->set_header(res, "Content-Type", "application/json");
    res->send(res, info);
    next(ctx);
}

int main() {
    printf("================================================================\n");
    printf("🧪 C-Express Streaming Framework - Test Suite\n");
    printf("================================================================\n\n");
    
    App app = create_app();
    
    app.get(&app, "/info", info_handler);
    app.post(&app, "/test/small", small_upload_handler);
    app.post(&app, "/test/large", large_upload_handler);
    app.post(&app, "/test/chunked", chunked_upload_handler);
    
    printf("🚀 Test endpoints:\n");
    printf("   GET  /info\n");
    printf("   POST /test/small\n");
    printf("   POST /test/large\n");
    printf("   POST /test/chunked\n\n");
    
    printf("🌐 Starting on port 8080...\n");
    printf("================================================================\n\n");
    
    app.listen(&app, 8080);
    return 0;
}
