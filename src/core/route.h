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

// Route metadata structures
typedef struct {
    char *name;                    // Parameter name
    ParameterType type;           // Parameter type
    int required;                 // Is parameter required?
    char *description;            // Parameter description
    char *example;                // Example value
    RouteConstraint *constraints; // Parameter constraints
} ParameterDoc;

typedef struct {
    int status_code;              // HTTP status code (200, 404, etc.)
    char *description;            // Response description
    char *content_type;           // Response content type
    char *example;                // Example response body
} ResponseDoc;

typedef struct {
    char *summary;                // Brief description
    char *description;            // Detailed description
    char **tags;                  // Array of tags (NULL-terminated)
    int tag_count;                // Number of tags
    ParameterDoc *parameters;     // Parameter documentation
    int param_doc_count;          // Number of documented parameters
    ResponseDoc *responses;       // Response documentation
    int response_count;           // Number of documented responses
    char *deprecated_reason;      // If route is deprecated
    char **examples;              // Example requests (NULL-terminated)
    int example_count;            // Number of examples
    void *custom_data;            // Extension point for custom metadata
} RouteMetadata;

// Enhanced route structure with metadata
typedef struct {
    RoutePattern *pattern;
    Layer layer;
    RouteMetadata *metadata;      // Route documentation/metadata
    char *route_id;               // Unique identifier for the route
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

// Route metadata functions
RouteMetadata* create_route_metadata(void);
void set_route_summary(RouteMetadata *metadata, const char *summary);
void set_route_description(RouteMetadata *metadata, const char *description);
void add_route_tag(RouteMetadata *metadata, const char *tag);
void add_parameter_doc(RouteMetadata *metadata, const char *name, ParameterType type, 
                       int required, const char *description, const char *example);
void add_response_doc(RouteMetadata *metadata, int status_code, const char *description,
                      const char *content_type, const char *example);
void set_route_deprecated(RouteMetadata *metadata, const char *reason);
void add_route_example(RouteMetadata *metadata, const char *example);
void free_route_metadata(RouteMetadata *metadata);

// Route documentation and introspection
char* generate_route_openapi_json(const Route *route, const char *base_path);
char* generate_routes_documentation(Route **routes, int route_count);
void print_route_info(const Route *route);
Route** find_routes_by_tag(Route **routes, int route_count, const char *tag, int *found_count);
Route* find_route_by_id(Route **routes, int route_count, const char *route_id);

#endif