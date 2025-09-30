#include "streaming.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>

#define _GNU_SOURCE

// Create a new stream context based on request headers
StreamContext* stream_create(int client_fd, const char *content_length_header, 
                           const char *transfer_encoding_header) {
    StreamContext *stream = malloc(sizeof(StreamContext));
    if (!stream) return NULL;
    
    // Initialize common fields
    memset(stream, 0, sizeof(StreamContext));
    stream->client_fd = client_fd;
    stream->buffer_pos = 0;
    stream->buffer_len = 0;
    stream->error = 0;
    
    // Determine body mode based on headers
    if (transfer_encoding_header && strstr(transfer_encoding_header, "chunked")) {
        // Chunked transfer encoding
        stream->mode = BODY_MODE_CHUNKED;
        stream->is_chunked = 1;
        stream->content_length = 0; // Unknown until all chunks read
        printf("[DEBUG] Stream: Using chunked transfer encoding\n");
    } else if (content_length_header) {
        // Content-Length specified
        stream->content_length = atoll(content_length_header);
        stream->is_chunked = 0;
        
        if (stream->content_length <= MAX_BODY_SIZE_SMALL) {
            // Small body - keep in memory
            stream->mode = BODY_MODE_MEMORY;
            stream->memory_capacity = stream->content_length + 1; // +1 for null terminator
            stream->memory_buffer = malloc(stream->memory_capacity);
            if (!stream->memory_buffer) {
                free(stream);
                return NULL;
            }
            printf("[DEBUG] Stream: Using memory mode for %zu bytes\n", stream->content_length);
        } else if (stream->content_length <= MAX_BODY_SIZE_LARGE) {
            // Large body - stream to temporary file
            stream->mode = BODY_MODE_STREAM;
            
            // Create temporary file
            snprintf(stream->temp_filename, sizeof(stream->temp_filename), 
                    "%s%ld_%d", TEMP_FILE_PREFIX, time(NULL), getpid());
            stream->temp_file = fopen(stream->temp_filename, "w+b");
            if (!stream->temp_file) {
                snprintf(stream->error_message, sizeof(stream->error_message),
                        "Failed to create temp file: %s", strerror(errno));
                stream->error = 1;
                free(stream);
                return NULL;
            }
            printf("[DEBUG] Stream: Using file mode for %zu bytes, temp file: %s\n", 
                   stream->content_length, stream->temp_filename);
        } else {
            // Body too large
            snprintf(stream->error_message, sizeof(stream->error_message),
                    "Request body too large: %zu bytes (max: %d)", 
                    stream->content_length, MAX_BODY_SIZE_LARGE);
            stream->error = 1;
            free(stream);
            return NULL;
        }
    } else {
        // No content length - assume small empty body
        stream->mode = BODY_MODE_MEMORY;
        stream->content_length = 0;
        stream->memory_capacity = 1;
        stream->memory_buffer = malloc(1);
        if (stream->memory_buffer) {
            stream->memory_buffer[0] = '\0';
        }
        printf("[DEBUG] Stream: No content length, using empty memory mode\n");
    }
    
    return stream;
}

// Read raw data from socket into internal buffer
static int stream_fill_buffer(StreamContext *stream) {
    if (stream->buffer_pos < stream->buffer_len) {
        return 0; // Buffer still has data
    }
    
    // Read new data from socket
    ssize_t bytes = recv(stream->client_fd, stream->read_buffer, 
                        STREAM_BUFFER_SIZE, MSG_DONTWAIT);
    
    if (bytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0; // No data available right now
        }
        snprintf(stream->error_message, sizeof(stream->error_message),
                "Socket read error: %s", strerror(errno));
        stream->error = 1;
        return -1;
    } else if (bytes == 0) {
        return 0; // Connection closed
    }
    
    stream->buffer_len = bytes;
    stream->buffer_pos = 0;
    return bytes;
}

// Parse chunk size from hex string
int parse_chunk_size(const char *line) {
    return (int)strtol(line, NULL, 16);
}

// Read a chunk of data for chunked encoding
static int stream_read_chunked_data(StreamContext *stream, char *buffer, 
                                  size_t buffer_size, size_t *bytes_read) {
    *bytes_read = 0;
    
    while (!stream->all_chunks_read && *bytes_read < buffer_size) {
        // If we need to read a new chunk header
        if (stream->current_chunk_size == 0 && stream->current_chunk_read == 0) {
            // Read chunk size line
            char chunk_line[32] = {0};
            size_t line_pos = 0;
            int found_crlf = 0;
            
            while (line_pos < sizeof(chunk_line) - 1 && !found_crlf) {
                if (stream->buffer_pos >= stream->buffer_len) {
                    int result = stream_fill_buffer(stream);
                    if (result < 0) return -1;
                    if (result == 0) break; // No more data
                }
                
                char c = stream->read_buffer[stream->buffer_pos++];
                if (c == '\r') {
                    // Look for \n
                    if (stream->buffer_pos >= stream->buffer_len) {
                        int result = stream_fill_buffer(stream);
                        if (result < 0) return -1;
                    }
                    if (stream->buffer_pos < stream->buffer_len && 
                        stream->read_buffer[stream->buffer_pos] == '\n') {
                        stream->buffer_pos++;
                        found_crlf = 1;
                    }
                } else if (c == '\n') {
                    found_crlf = 1;
                } else {
                    chunk_line[line_pos++] = c;
                }
            }
            
            if (!found_crlf) {
                snprintf(stream->error_message, sizeof(stream->error_message),
                        "Invalid chunk header");
                stream->error = 1;
                return -1;
            }
            
            stream->current_chunk_size = parse_chunk_size(chunk_line);
            stream->current_chunk_read = 0;
            
            printf("[DEBUG] Stream: New chunk size: %zu\n", stream->current_chunk_size);
            
            if (stream->current_chunk_size == 0) {
                // Final chunk
                stream->all_chunks_read = 1;
                stream->chunk_complete = 1;
                break;
            }
        }
        
        // Read chunk data
        if (stream->current_chunk_read < stream->current_chunk_size) {
            size_t to_read = stream->current_chunk_size - stream->current_chunk_read;
            size_t buffer_space = buffer_size - *bytes_read;
            size_t available = stream->buffer_len - stream->buffer_pos;
            
            if (available == 0) {
                int result = stream_fill_buffer(stream);
                if (result < 0) return -1;
                if (result == 0) break;
                available = stream->buffer_len - stream->buffer_pos;
            }
            
            size_t copy_size = (to_read < buffer_space) ? to_read : buffer_space;
            copy_size = (copy_size < available) ? copy_size : available;
            
            memcpy(buffer + *bytes_read, stream->read_buffer + stream->buffer_pos, copy_size);
            *bytes_read += copy_size;
            stream->buffer_pos += copy_size;
            stream->current_chunk_read += copy_size;
            stream->bytes_read += copy_size;
            
            // Check if chunk is complete
            if (stream->current_chunk_read >= stream->current_chunk_size) {
                // Skip trailing CRLF
                while (stream->buffer_pos < stream->buffer_len) {
                    char c = stream->read_buffer[stream->buffer_pos++];
                    if (c == '\n') break;
                }
                
                stream->current_chunk_size = 0;
                stream->current_chunk_read = 0;
                stream->chunk_complete = 1;
            }
        }
    }
    
    return 0;
}

// Read a chunk of data from the stream
int stream_read_chunk(StreamContext *stream, char *buffer, size_t buffer_size, size_t *bytes_read) {
    if (!stream || !buffer || !bytes_read) return -1;
    
    *bytes_read = 0;
    
    if (stream->error) return -1;
    if (stream_is_complete(stream)) return 0;
    
    if (stream->mode == BODY_MODE_CHUNKED) {
        return stream_read_chunked_data(stream, buffer, buffer_size, bytes_read);
    }
    
    // For content-length based reading
    size_t remaining = stream->content_length - stream->bytes_read;
    if (remaining == 0) return 0;
    
    size_t to_read = (remaining < buffer_size) ? remaining : buffer_size;
    
    // Fill buffer if needed
    if (stream->buffer_pos >= stream->buffer_len) {
        int result = stream_fill_buffer(stream);
        if (result < 0) return -1;
        if (result == 0 && remaining > 0) {
            snprintf(stream->error_message, sizeof(stream->error_message),
                    "Unexpected end of stream, expected %zu more bytes", remaining);
            stream->error = 1;
            return -1;
        }
    }
    
    size_t available = stream->buffer_len - stream->buffer_pos;
    size_t copy_size = (to_read < available) ? to_read : available;
    
    if (copy_size > 0) {
        memcpy(buffer, stream->read_buffer + stream->buffer_pos, copy_size);
        *bytes_read = copy_size;
        stream->buffer_pos += copy_size;
        stream->bytes_read += copy_size;
    }
    
    return 0;
}

// Read all data using a callback function
int stream_read_all(StreamContext *stream, StreamCallback callback, void *user_data) {
    if (!stream || !callback) return -1;
    
    char buffer[STREAM_BUFFER_SIZE];
    size_t bytes_read;
    
    while (!stream_is_complete(stream) && !stream->error) {
        int result = stream_read_chunk(stream, buffer, sizeof(buffer), &bytes_read);
        if (result < 0) return -1;
        
        if (bytes_read > 0) {
            // Process data based on mode
            if (stream->mode == BODY_MODE_MEMORY) {
                // Append to memory buffer
                size_t new_size = stream->bytes_read;
                if (new_size >= stream->memory_capacity) {
                    // Expand buffer
                    stream->memory_capacity = new_size * 2;
                    char *new_buffer = realloc(stream->memory_buffer, stream->memory_capacity);
                    if (!new_buffer) {
                        snprintf(stream->error_message, sizeof(stream->error_message),
                                "Memory allocation failed");
                        stream->error = 1;
                        return -1;
                    }
                    stream->memory_buffer = new_buffer;
                }
                
                memcpy(stream->memory_buffer + stream->bytes_read - bytes_read, 
                       buffer, bytes_read);
                stream->memory_buffer[stream->bytes_read] = '\0';
                
            } else if (stream->mode == BODY_MODE_STREAM) {
                // Write to temporary file
                if (fwrite(buffer, 1, bytes_read, stream->temp_file) != bytes_read) {
                    snprintf(stream->error_message, sizeof(stream->error_message),
                            "Failed to write to temp file: %s", strerror(errno));
                    stream->error = 1;
                    return -1;
                }
            }
            
            // Call user callback
            if (callback(buffer, bytes_read, user_data) != 0) {
                return -1; // User requested abort
            }
        }
        
        if (bytes_read == 0) {
            // No more data available right now
            break;
        }
    }
    
    return 0;
}

// Check if stream is complete
int stream_is_complete(StreamContext *stream) {
    if (!stream) return 1;
    
    if (stream->mode == BODY_MODE_CHUNKED) {
        return stream->all_chunks_read;
    }
    
    return stream->bytes_read >= stream->content_length;
}

// Check if stream has error
int stream_has_error(StreamContext *stream) {
    return stream ? stream->error : 1;
}

// Get error message
const char* stream_get_error(StreamContext *stream) {
    return stream ? stream->error_message : "Invalid stream";
}

// Get memory content (for small bodies)
const char* stream_get_memory_content(StreamContext *stream) {
    if (!stream || stream->mode != BODY_MODE_MEMORY) return NULL;
    return stream->memory_buffer;
}

// Get content length
size_t stream_get_content_length(StreamContext *stream) {
    return stream ? stream->bytes_read : 0;
}

// Get temporary file path
const char* stream_get_temp_file(StreamContext *stream) {
    if (!stream || stream->mode != BODY_MODE_STREAM) return NULL;
    return stream->temp_filename;
}

// Get file handle
FILE* stream_get_file_handle(StreamContext *stream) {
    if (!stream || stream->mode != BODY_MODE_STREAM) return NULL;
    return stream->temp_file;
}

// Save stream content to a file
int stream_save_to_file(StreamContext *stream, const char *filename) {
    if (!stream || !filename) return -1;
    
    FILE *output = fopen(filename, "wb");
    if (!output) return -1;
    
    int result = 0;
    
    if (stream->mode == BODY_MODE_MEMORY && stream->memory_buffer) {
        if (fwrite(stream->memory_buffer, 1, stream->bytes_read, output) != stream->bytes_read) {
            result = -1;
        }
    } else if (stream->mode == BODY_MODE_STREAM && stream->temp_file) {
        // Copy from temp file to output file
        rewind(stream->temp_file);
        char buffer[STREAM_BUFFER_SIZE];
        size_t bytes;
        
        while ((bytes = fread(buffer, 1, sizeof(buffer), stream->temp_file)) > 0) {
            if (fwrite(buffer, 1, bytes, output) != bytes) {
                result = -1;
                break;
            }
        }
    } else {
        result = -1;
    }
    
    fclose(output);
    return result;
}

// Copy stream content to a buffer
int stream_copy_to_buffer(StreamContext *stream, char *buffer, size_t buffer_size) {
    if (!stream || !buffer || buffer_size == 0) return -1;
    
    if (stream->mode == BODY_MODE_MEMORY && stream->memory_buffer) {
        size_t copy_size = (stream->bytes_read < buffer_size - 1) ? 
                          stream->bytes_read : buffer_size - 1;
        memcpy(buffer, stream->memory_buffer, copy_size);
        buffer[copy_size] = '\0';
        return 0;
    } else if (stream->mode == BODY_MODE_STREAM && stream->temp_file) {
        rewind(stream->temp_file);
        size_t bytes_read = fread(buffer, 1, buffer_size - 1, stream->temp_file);
        buffer[bytes_read] = '\0';
        return 0;
    }
    
    return -1;
}

// Cleanup stream resources
void stream_destroy(StreamContext *stream) {
    if (!stream) return;
    
    if (stream->memory_buffer) {
        free(stream->memory_buffer);
    }
    
    if (stream->temp_file) {
        fclose(stream->temp_file);
        unlink(stream->temp_filename); // Delete temporary file
    }
    
    free(stream);
}
