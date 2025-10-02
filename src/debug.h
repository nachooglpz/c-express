#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

// Debug configuration
#ifdef C_EXPRESS_DEBUG
    // For messages with no format arguments
    #define DEBUG_PRINT_STR(msg) fprintf(stderr, "[C-Express DEBUG] " msg)
    // For messages with format arguments
    #define DEBUG_PRINT(fmt, ...) fprintf(stderr, "[C-Express DEBUG] " fmt, __VA_ARGS__)
    #define DEBUG_ENABLED 1
#else
    #define DEBUG_PRINT_STR(msg) ((void)0)
    #define DEBUG_PRINT(fmt, ...) ((void)0)
    #define DEBUG_ENABLED 0
#endif

// Error logging (always enabled for important errors)
#define ERROR_PRINT_STR(msg) fprintf(stderr, "[C-Express ERROR] " msg)
#define ERROR_PRINT(fmt, ...) fprintf(stderr, "[C-Express ERROR] " fmt, __VA_ARGS__)

// Info logging (can be controlled separately)
#ifdef C_EXPRESS_VERBOSE
    #define INFO_PRINT_STR(msg) fprintf(stdout, "[C-Express] " msg)
    #define INFO_PRINT(fmt, ...) fprintf(stdout, "[C-Express] " fmt, __VA_ARGS__)
#else
    #define INFO_PRINT(fmt, ...) ((void)0)
#endif

#endif // DEBUG_H
