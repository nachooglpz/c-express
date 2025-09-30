#ifndef STREAMING_H
#define STREAMING_H

#include <stddef.h>
#include <stdio.h>

// Configuration constants
#define STREAM_BUFFER_SIZE 8192        // 8KB buffer for streaming
#define MAX_BODY_SIZE_SMALL 16384      // 16KB for small bodies (keep in memory)
#define MAX_BODY_SIZE_LARGE 104857600  // 100MB max for large bodies (stream to disk)
#define TEMP_FILE_PREFIX "/tmp/c_express_"

// Body handling modes
typedef enum {
    BODY_MODE_MEMORY,     // Small body, keep in memory
    BODY_MODE_STREAM,     // Large body, stream to temporary file
    BODY_MODE_CHUNKED     // Chunked transfer encoding
} BodyMode;

// Stream state for reading large bodies
typedef struct {
    BodyMode mode;
    size_t content_length;
    size_t bytes_read;
    int is_chunked;
    int client_fd;
    
    // For memory mode
    char *memory_buffer;
    size_t memory_capacity;
    
    // For streaming mode
    FILE *temp_file;
    char temp_filename[256];
    
    // For chunked mode
    size_t current_chunk_size;
    size_t current_chunk_read;
    int chunk_complete;
    int all_chunks_read;
    
    // Buffer for reading data
    char read_buffer[STREAM_BUFFER_SIZE];
    size_t buffer_pos;
    size_t buffer_len;
    
    // Error handling
    int error;
    char error_message[256];
} StreamContext;

// Callback function type for streaming data
typedef int (*StreamCallback)(const char *data, size_t length, void *user_data);

// Function declarations
StreamContext* stream_create(int client_fd, const char *content_length_header, 
                           const char *transfer_encoding_header);

int stream_read_chunk(StreamContext *stream, char *buffer, size_t buffer_size, size_t *bytes_read);
int stream_read_all(StreamContext *stream, StreamCallback callback, void *user_data);
int stream_is_complete(StreamContext *stream);
int stream_has_error(StreamContext *stream);
const char* stream_get_error(StreamContext *stream);

// Get body content (for small bodies in memory)
const char* stream_get_memory_content(StreamContext *stream);
size_t stream_get_content_length(StreamContext *stream);

// Get temporary file path (for large bodies)
const char* stream_get_temp_file(StreamContext *stream);
FILE* stream_get_file_handle(StreamContext *stream);

// Cleanup
void stream_destroy(StreamContext *stream);

// Utility functions
int stream_save_to_file(StreamContext *stream, const char *filename);
int stream_copy_to_buffer(StreamContext *stream, char *buffer, size_t buffer_size);

// Chunked transfer encoding helpers
int parse_chunk_size(const char *line);
int read_chunk_data(int client_fd, char *buffer, size_t chunk_size);

#endif
