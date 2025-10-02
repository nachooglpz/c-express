// Helper structure for building JSON strings
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *buffer;
    size_t size;
    size_t capacity;
} JsonStringBuilder;

static JsonStringBuilder* json_builder_create(size_t initial_capacity) {
    JsonStringBuilder *builder = malloc(sizeof(JsonStringBuilder));
    if (!builder) return NULL;
    
    builder->buffer = malloc(initial_capacity);
    if (!builder->buffer) {
        free(builder);
        return NULL;
    }
    
    builder->size = 0;
    builder->capacity = initial_capacity;
    builder->buffer[0] = '\0';
    
    return builder;
}

static void json_builder_append(JsonStringBuilder *builder, const char *str) {
    if (!builder || !str) return;
    
    size_t str_len = strlen(str);
    size_t new_size = builder->size + str_len;
    
    // Resize if needed
    if (new_size >= builder->capacity) {
        size_t new_capacity = builder->capacity * 2;
        while (new_capacity <= new_size) new_capacity *= 2;
        
        char *new_buffer = realloc(builder->buffer, new_capacity);
        if (!new_buffer) return; // Failed to resize
        
        builder->buffer = new_buffer;
        builder->capacity = new_capacity;
    }
    
    strcat(builder->buffer, str);
    builder->size = new_size;
}

static void json_builder_append_format(JsonStringBuilder *builder, const char *format, ...) {
    if (!builder || !format) return;
    
    va_list args;
    va_start(args, format);
    
    // Calculate required size
    va_list args_copy;
    va_copy(args_copy, args);
    int required_size = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);
    
    if (required_size < 0) {
        va_end(args);
        return;
    }
    
    // Resize if needed
    size_t new_size = builder->size + required_size;
    if (new_size >= builder->capacity) {
        size_t new_capacity = builder->capacity * 2;
        while (new_capacity <= new_size) new_capacity *= 2;
        
        char *new_buffer = realloc(builder->buffer, new_capacity);
        if (!new_buffer) {
            va_end(args);
            return;
        }
        
        builder->buffer = new_buffer;
        builder->capacity = new_capacity;
    }
    
    // Append formatted string
    vsnprintf(builder->buffer + builder->size, builder->capacity - builder->size, format, args);
    builder->size = new_size;
    
    va_end(args);
}

static char* json_builder_finalize(JsonStringBuilder *builder) {
    if (!builder) return NULL;
    
    char *result = builder->buffer;
    free(builder);
    return result;
}
