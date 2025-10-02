#include "error.h"
#include "../debug.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char *error_get_status_text(ErrorCode code) {
    switch (code) {
        case ERROR_NONE: return "OK";
        case ERROR_BAD_REQUEST: return "Bad Request";
        case ERROR_UNAUTHORIZED: return "Unauthorized";
        case ERROR_FORBIDDEN: return "Forbidden";
        case ERROR_NOT_FOUND: return "Not Found";
        case ERROR_METHOD_NOT_ALLOWED: return "Method Not Allowed";
        case ERROR_CONFLICT: return "Conflict";
        case ERROR_UNPROCESSABLE_ENTITY: return "Unprocessable Entity";
        case ERROR_INTERNAL_SERVER_ERROR: return "Internal Server Error";
        case ERROR_NOT_IMPLEMENTED: return "Not Implemented";
        case ERROR_BAD_GATEWAY: return "Bad Gateway";
        case ERROR_SERVICE_UNAVAILABLE: return "Service Unavailable";
        default: return "Unknown Error";
    }
}

Error *create_error(ErrorCode code, const char *message, const char *file, int line, const char *function) {
    Error *error = malloc(sizeof(Error));
    if (!error) return NULL;
    
    error->code = code;
    error->status_code = (int)code;
    error->is_handled = false;
    error->file = file;
    error->line = line;
    error->function = function;
    
    // Set message
    if (message) {
        strncpy(error->message, message, sizeof(error->message) - 1);
        error->message[sizeof(error->message) - 1] = '\0';
    } else {
        strncpy(error->message, error_get_status_text(code), sizeof(error->message) - 1);
        error->message[sizeof(error->message) - 1] = '\0';
    }
    
    // Generate stack trace info
    snprintf(error->stack_trace, sizeof(error->stack_trace),
        "Error occurred in %s() at %s:%d\n", function, file, line);
    
    ERROR_PRINT("%s (%d): %s\n", error_get_status_text(code), error->status_code, error->message);
    ERROR_PRINT("Stack trace: %s", error->stack_trace);
    
    return error;
}

void destroy_error(Error *error) {
    if (error) {
        free(error);
    }
}

void error_set_message(Error *error, const char *message) {
    if (error && message) {
        strncpy(error->message, message, sizeof(error->message) - 1);
        error->message[sizeof(error->message) - 1] = '\0';
    }
}

void error_set_stack_trace(Error *error, const char *stack_trace) {
    if (error && stack_trace) {
        strncpy(error->stack_trace, stack_trace, sizeof(error->stack_trace) - 1);
        error->stack_trace[sizeof(error->stack_trace) - 1] = '\0';
    }
}

ErrorContext *create_error_context() {
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    if (!ctx) return NULL;
    
    ctx->current_error = NULL;
    ctx->has_error = false;
    ctx->error_handler = NULL;
    
    return ctx;
}

void destroy_error_context(ErrorContext *ctx) {
    if (ctx) {
        if (ctx->current_error) {
            destroy_error(ctx->current_error);
        }
        free(ctx);
    }
}

void error_context_set_error(ErrorContext *ctx, Error *error) {
    if (!ctx) return;
    
    // Clean up previous error
    if (ctx->current_error) {
        destroy_error(ctx->current_error);
    }
    
    ctx->current_error = error;
    ctx->has_error = (error != NULL);
    
    // Call error handler if set
    if (ctx->error_handler && error) {
        ctx->error_handler(error, ctx);
    }
}

void error_context_clear(ErrorContext *ctx) {
    if (ctx) {
        if (ctx->current_error) {
            destroy_error(ctx->current_error);
            ctx->current_error = NULL;
        }
        ctx->has_error = false;
    }
}

bool error_context_has_error(ErrorContext *ctx) {
    return ctx && ctx->has_error && ctx->current_error;
}
