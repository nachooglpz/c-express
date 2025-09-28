#ifndef ERROR_H
#define ERROR_H

#include <stdbool.h>

#define MAX_ERROR_MESSAGE 512
#define MAX_STACK_TRACE 1024

typedef enum {
    ERROR_NONE = 0,
    ERROR_BAD_REQUEST = 400,
    ERROR_UNAUTHORIZED = 401,
    ERROR_FORBIDDEN = 403,
    ERROR_NOT_FOUND = 404,
    ERROR_METHOD_NOT_ALLOWED = 405,
    ERROR_CONFLICT = 409,
    ERROR_UNPROCESSABLE_ENTITY = 422,
    ERROR_INTERNAL_SERVER_ERROR = 500,
    ERROR_NOT_IMPLEMENTED = 501,
    ERROR_BAD_GATEWAY = 502,
    ERROR_SERVICE_UNAVAILABLE = 503
} ErrorCode;

typedef struct {
    ErrorCode code;
    int status_code;
    char message[MAX_ERROR_MESSAGE];
    char stack_trace[MAX_STACK_TRACE];
    bool is_handled;
    const char *file;
    int line;
    const char *function;
} Error;

// Error handling context
typedef struct {
    Error *current_error;
    bool has_error;
    void (*error_handler)(Error *error, void *context);
} ErrorContext;

// Error handling functions
Error *create_error(ErrorCode code, const char *message, const char *file, int line, const char *function);
void destroy_error(Error *error);
void error_set_message(Error *error, const char *message);
void error_set_stack_trace(Error *error, const char *stack_trace);
const char *error_get_status_text(ErrorCode code);

// Error context functions
ErrorContext *create_error_context();
void destroy_error_context(ErrorContext *ctx);
void error_context_set_error(ErrorContext *ctx, Error *error);
void error_context_clear(ErrorContext *ctx);
bool error_context_has_error(ErrorContext *ctx);

// Macros for easier error handling
#define THROW_ERROR(ctx, code, msg) do { \
    Error *_err = create_error(code, msg, __FILE__, __LINE__, __func__); \
    error_context_set_error(ctx, _err); \
} while(0)

#define TRY(ctx) if (!error_context_has_error(ctx)) {

#define CATCH(ctx) } if (error_context_has_error(ctx)) {

#define FINALLY(ctx) }

#endif
