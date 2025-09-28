#ifndef ROUTE_H
#define ROUTE_H

#include "layer.h"

// Route parameter types
typedef enum {
    PARAM_STRING,
    PARAM_NUMBER,
    PARAM_SLUG,
    PARAM_UUID,
    PARAM_ANY
} ParameterType;

// Route parameter definition
typedef struct {
    char *name;
    ParameterType type;
    int is_optional;
    char *value;  // Extracted value during matching
} RouteParam;

// Route pattern segment types
typedef enum {
    SEGMENT_LITERAL,      // /users
    SEGMENT_PARAMETER,    // /:id
    SEGMENT_WILDCARD,     // /*
    SEGMENT_OPTIONAL      // /:id?
} SegmentType;

// Route pattern segment
typedef struct {
    SegmentType type;
    char *literal_value;  // For SEGMENT_LITERAL
    RouteParam *param;    // For parameter segments
} RouteSegment;

// Compiled route pattern
typedef struct {
    char *original_pattern;
    RouteSegment *segments;
    int segment_count;
    int priority;         // Lower number = higher priority
    int has_wildcards;
    int param_count;
} RoutePattern;

// Route matching result
typedef struct {
    int matched;
    RouteParam *params;
    int param_count;
    char *wildcard_path;  // Captured wildcard portion
} RouteMatch;

// Route structure
typedef struct {
    RoutePattern *pattern;
    Layer layer;
} Route;

// Function declarations
RoutePattern *compile_route_pattern(const char *pattern);
RouteMatch route_pattern_match(RoutePattern *pattern, const char *path);
void free_route_pattern(RoutePattern *pattern);
void free_route_match(RouteMatch *match);
RouteMatch* duplicate_route_match(const RouteMatch* original);
int validate_parameter_value(const char *value, ParameterType type);
int calculate_route_priority(RoutePattern *pattern);

#endif