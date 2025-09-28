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

// Constraint types
typedef enum {
    CONSTRAINT_MIN,           // Minimum value/length
    CONSTRAINT_MAX,           // Maximum value/length
    CONSTRAINT_RANGE,         // Value/length range
    CONSTRAINT_REGEX,         // Regular expression pattern
    CONSTRAINT_ENUM,          // Must be one of specified values
    CONSTRAINT_CUSTOM         // Custom validation function
} ConstraintType;

// Validation error info
typedef struct {
    char *param_name;
    char *error_message;
    char *provided_value;
    ConstraintType failed_constraint;
} ValidationError;

// Custom validator function type
typedef int (*CustomValidator)(const char *value, void *context, ValidationError *error);

// Route constraint definition
typedef struct RouteConstraint {
    ConstraintType type;
    union {
        struct {
            long min_value;
            long max_value;
        } range;
        char *regex_pattern;
        char **enum_values;      // NULL-terminated array
        CustomValidator validator;
    } constraint;
    char *error_message;         // Custom error message
    void *context;              // Context for custom validators
    struct RouteConstraint *next; // For constraint chaining
} RouteConstraint;

// Enhanced route parameter definition
typedef struct {
    char *name;
    ParameterType type;
    int is_optional;
    char *value;                 // Extracted value during matching
    RouteConstraint *constraints; // Linked list of constraints
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

// Constraint and validation functions
RouteConstraint* create_min_constraint(long min_value, const char *error_msg);
RouteConstraint* create_max_constraint(long max_value, const char *error_msg);
RouteConstraint* create_range_constraint(long min_value, long max_value, const char *error_msg);
RouteConstraint* create_regex_constraint(const char *pattern, const char *error_msg);
RouteConstraint* create_enum_constraint(const char **values, const char *error_msg);
RouteConstraint* create_custom_constraint(CustomValidator validator, void *context, const char *error_msg);

// Constraint application and validation
void add_parameter_constraint(RouteParam *param, RouteConstraint *constraint);
int validate_parameter_constraints(const RouteParam *param, ValidationError *error);
int validate_route_match(const RouteMatch *match, ValidationError **errors, int *error_count);

// Constraint utilities
void free_constraint(RouteConstraint *constraint);
void free_validation_error(ValidationError *error);
RoutePattern* compile_route_pattern_with_constraints(const char *pattern);

// Internal constraint parsing functions
ParameterType parse_parameter_type_and_constraints(char *param_name, RouteParam *param);
void parse_parameter_constraints(const char *constraint_str, RouteParam *param);

#endif