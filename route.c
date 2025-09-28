#include "route.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

// Helper function to split path into segments
char **split_path_segments(const char *path, int *count) {
    if (!path || strlen(path) == 0) {
        *count = 0;
        return NULL;
    }
    
    // Handle root path "/"
    if (strcmp(path, "/") == 0) {
        char **segments = malloc(sizeof(char*));
        segments[0] = strdup("");
        *count = 1;
        return segments;
    }
    
    // Skip leading slash
    const char *start = path;
    if (*start == '/') start++;
    
    // First pass: count actual segments
    int segment_count = 0;
    const char *p = start;
    const char *seg_start = start;
    
    while (*p) {
        if (*p == '/') {
            if (p > seg_start) {  // Non-empty segment
                segment_count++;
            }
            seg_start = p + 1;
        }
        p++;
    }
    
    // Count the final segment if non-empty
    if (p > seg_start) {
        segment_count++;
    }
    
    if (segment_count == 0) {
        *count = 0;
        return NULL;
    }
    
    // Second pass: extract segments
    char **segments = malloc(segment_count * sizeof(char*));
    int segment_idx = 0;
    p = start;
    seg_start = start;
    
    while (*p) {
        if (*p == '/') {
            if (p > seg_start) {  // Non-empty segment
                int len = p - seg_start;
                segments[segment_idx] = malloc(len + 1);
                strncpy(segments[segment_idx], seg_start, len);
                segments[segment_idx][len] = '\0';
                segment_idx++;
            }
            seg_start = p + 1;
        }
        p++;
    }
    
    // Add final segment if non-empty
    if (p > seg_start) {
        int len = p - seg_start;
        segments[segment_idx] = malloc(len + 1);
        strncpy(segments[segment_idx], seg_start, len);
        segments[segment_idx][len] = '\0';
        segment_idx++;
    }
    
    *count = segment_count;
    return segments;
}

// Check if a segment is a parameter (:param or :param?)
int is_parameter_segment(const char *segment, char **param_name, int *is_optional) {
    if (!segment || segment[0] != ':') return 0;
    
    int len = strlen(segment);
    *is_optional = (len > 1 && segment[len - 1] == '?');
    
    int name_len = len - 1;
    if (*is_optional) name_len--;
    
    *param_name = malloc(name_len + 1);
    strncpy(*param_name, segment + 1, name_len);
    (*param_name)[name_len] = '\0';
    
    return 1;
}

// Enhanced parameter parsing with constraints
ParameterType parse_parameter_type_and_constraints(char *param_name, RouteParam *param) {
    char *colon = strchr(param_name, ':');
    if (!colon) return PARAM_STRING;
    
    *colon = '\0';  // Split name and type
    char *type_and_constraints = colon + 1;
    
    // Parse type (first part after colon)
    char *constraint_start = strchr(type_and_constraints, '(');
    ParameterType type = PARAM_STRING;
    
    if (constraint_start) {
        *constraint_start = '\0';
        constraint_start++; // Move past opening parenthesis
    }
    
    // Determine parameter type
    if (strcmp(type_and_constraints, "number") == 0) type = PARAM_NUMBER;
    else if (strcmp(type_and_constraints, "slug") == 0) type = PARAM_SLUG;
    else if (strcmp(type_and_constraints, "uuid") == 0) type = PARAM_UUID;
    else if (strcmp(type_and_constraints, "any") == 0) type = PARAM_ANY;
    
    // Parse constraints if present
    if (constraint_start) {
        parse_parameter_constraints(constraint_start, param);
    }
    
    return type;
}

// Parse constraints from constraint string
void parse_parameter_constraints(const char *constraint_str, RouteParam *param) {
    if (!constraint_str || !param) return;
    
    char *constraints_copy = strdup(constraint_str);
    char *constraint = strtok(constraints_copy, ",");
    
    while (constraint) {
        // Trim whitespace
        while (*constraint == ' ') constraint++;
        
        // Parse different constraint types
        if (strncmp(constraint, "min=", 4) == 0) {
            long min_val = strtol(constraint + 4, NULL, 10);
            RouteConstraint *c = create_min_constraint(min_val, NULL);
            add_parameter_constraint(param, c);
            
        } else if (strncmp(constraint, "max=", 4) == 0) {
            long max_val = strtol(constraint + 4, NULL, 10);
            RouteConstraint *c = create_max_constraint(max_val, NULL);
            add_parameter_constraint(param, c);
            
        } else if (strncmp(constraint, "range=", 6) == 0) {
            char *range_str = constraint + 6;
            char *dash = strchr(range_str, '-');
            if (dash) {
                *dash = '\0';
                long min_val = strtol(range_str, NULL, 10);
                long max_val = strtol(dash + 1, NULL, 10);
                RouteConstraint *c = create_range_constraint(min_val, max_val, NULL);
                add_parameter_constraint(param, c);
            }
            
        } else if (strncmp(constraint, "regex=", 6) == 0) {
            RouteConstraint *c = create_regex_constraint(constraint + 6, NULL);
            add_parameter_constraint(param, c);
            
        } else if (strncmp(constraint, "enum=", 5) == 0) {
            // Parse enum values (separated by |)
            char *enum_str = constraint + 5;
            // Count values
            int count = 1;
            for (char *p = enum_str; *p; p++) {
                if (*p == '|') count++;
            }
            
            const char **values = malloc((count + 1) * sizeof(char*));
            char *enum_copy = strdup(enum_str);
            char *value = strtok(enum_copy, "|");
            int i = 0;
            while (value && i < count) {
                values[i++] = strdup(value);
                value = strtok(NULL, "|");
            }
            values[i] = NULL;
            
            RouteConstraint *c = create_enum_constraint(values, NULL);
            add_parameter_constraint(param, c);
            
            // Cleanup
            for (int j = 0; j < i; j++) {
                free((char*)values[j]);
            }
            free(values);
            free(enum_copy);
        }
        
        constraint = strtok(NULL, ",");
    }
    
    free(constraints_copy);
}

// Legacy function for backward compatibility
ParameterType parse_parameter_type(char *param_name) {
    char *name_copy = strdup(param_name);
    RouteParam dummy_param = {0};
    ParameterType type = parse_parameter_type_and_constraints(name_copy, &dummy_param);
    free(name_copy);
    return type;
}

// Validate parameter value based on type
int validate_parameter_value(const char *value, ParameterType type) {
    if (!value) return 0;
    
    switch (type) {
        case PARAM_NUMBER: {
            char *endptr;
            strtol(value, &endptr, 10);
            return *endptr == '\0';  // Valid if entire string was consumed
        }
        
        case PARAM_SLUG: {
            // Slug: alphanumeric, hyphens, underscores
            for (const char *p = value; *p; p++) {
                if (!isalnum(*p) && *p != '-' && *p != '_') return 0;
            }
            return strlen(value) > 0;
        }
        
        case PARAM_UUID: {
            // Simple UUID pattern check
            regex_t regex;
            int ret = regcomp(&regex, "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$", REG_EXTENDED);
            if (ret) return 0;
            
            ret = regexec(&regex, value, 0, NULL, 0);
            regfree(&regex);
            return ret == 0;
        }
        
        case PARAM_STRING:
        case PARAM_ANY:
        default:
            return strlen(value) > 0;
    }
}

// Calculate route priority (lower = higher priority)
int calculate_route_priority(RoutePattern *pattern) {
    if (!pattern) return 1000;
    
    int priority = 0;
    
    for (int i = 0; i < pattern->segment_count; i++) {
        RouteSegment *seg = &pattern->segments[i];
        
        switch (seg->type) {
            case SEGMENT_LITERAL:
                priority += 0;  // Highest priority
                break;
            case SEGMENT_PARAMETER:
                priority += seg->param->is_optional ? 20 : 10;
                break;
            case SEGMENT_WILDCARD:
                priority += 100;  // Lowest priority
                break;
            case SEGMENT_OPTIONAL:
                priority += 15;
                break;
        }
    }
    
    // Prefer routes with more literal segments
    priority -= pattern->segment_count * 2;
    
    return priority;
}

// Compile route pattern into optimized structure
RoutePattern *compile_route_pattern(const char *pattern) {
    if (!pattern) return NULL;
    
    printf("[DEBUG] compile_route_pattern: compiling '%s'\n", pattern);
    
    RoutePattern *route_pattern = malloc(sizeof(RoutePattern));
    route_pattern->original_pattern = strdup(pattern);
    route_pattern->has_wildcards = 0;
    route_pattern->param_count = 0;
    
    // Split pattern into segments
    int segment_count;
    char **path_segments = split_path_segments(pattern, &segment_count);
    
    route_pattern->segments = malloc(segment_count * sizeof(RouteSegment));
    route_pattern->segment_count = segment_count;
    
    for (int i = 0; i < segment_count; i++) {
        RouteSegment *seg = &route_pattern->segments[i];
        char *segment = path_segments[i];
        
        if (strcmp(segment, "*") == 0) {
            // Wildcard segment
            seg->type = SEGMENT_WILDCARD;
            seg->literal_value = NULL;
            seg->param = NULL;
            route_pattern->has_wildcards = 1;
            printf("[DEBUG] compile_route_pattern: wildcard segment at %d\n", i);
            
        } else if (segment[0] == ':') {
            // Parameter segment
            char *param_name;
            int is_optional;
            
            if (is_parameter_segment(segment, &param_name, &is_optional)) {
                seg->type = is_optional ? SEGMENT_OPTIONAL : SEGMENT_PARAMETER;
                seg->literal_value = NULL;
                
                seg->param = malloc(sizeof(RouteParam));
                seg->param->is_optional = is_optional;
                seg->param->value = NULL;
                seg->param->constraints = NULL;
                
                // Parse type and constraints - this will modify param_name to extract just the name part
                char *param_copy = strdup(param_name);
                seg->param->type = parse_parameter_type_and_constraints(param_copy, seg->param);
                
                // Set the name to just the parameter name (without type suffix)
                seg->param->name = strdup(param_copy);
                
                free(param_copy);
                
                route_pattern->param_count++;
                printf("[DEBUG] compile_route_pattern: param '%s' (type=%d, optional=%d) at %d\n", 
                       seg->param->name, seg->param->type, is_optional, i);
                
                free(param_name);
            } else {
                // Treat as literal if parsing failed
                seg->type = SEGMENT_LITERAL;
                seg->literal_value = strdup(segment);
                seg->param = NULL;
            }
            
        } else {
            // Literal segment
            seg->type = SEGMENT_LITERAL;
            seg->literal_value = strdup(segment);
            seg->param = NULL;
            printf("[DEBUG] compile_route_pattern: literal '%s' at %d\n", segment, i);
        }
    }
    
    // Calculate priority
    route_pattern->priority = calculate_route_priority(route_pattern);
    printf("[DEBUG] compile_route_pattern: pattern '%s' priority=%d\n", pattern, route_pattern->priority);
    
    // Cleanup
    for (int i = 0; i < segment_count; i++) {
        free(path_segments[i]);
    }
    free(path_segments);
    
    return route_pattern;
}

// Match path against compiled route pattern
RouteMatch route_pattern_match(RoutePattern *pattern, const char *path) {
    RouteMatch match = { 0 };
    
    if (!pattern || !path) {
        return match;
    }
    
    printf("[DEBUG] route_pattern_match: matching '%s' against pattern '%s'\n", path, pattern->original_pattern);
    
    // Split incoming path into segments
    int path_segment_count;
    char **path_segments = split_path_segments(path, &path_segment_count);
    
    // Prepare match result
    match.params = NULL;
    match.param_count = 0;
    match.wildcard_path = NULL;
    
    if (pattern->param_count > 0) {
        match.params = malloc(pattern->param_count * sizeof(RouteParam));
    }
    
    int path_idx = 0;
    int param_idx = 0;
    
    for (int pattern_idx = 0; pattern_idx < pattern->segment_count; pattern_idx++) {
        RouteSegment *seg = &pattern->segments[pattern_idx];
        
        switch (seg->type) {
            case SEGMENT_LITERAL: {
                if (path_idx >= path_segment_count) {
                    printf("[DEBUG] route_pattern_match: path too short for literal segment\n");
                    goto no_match;
                }
                
                if (strcmp(seg->literal_value, path_segments[path_idx]) != 0) {
                    printf("[DEBUG] route_pattern_match: literal mismatch '%s' != '%s'\n", 
                           seg->literal_value, path_segments[path_idx]);
                    goto no_match;
                }
                path_idx++;
                break;
            }
            
            case SEGMENT_PARAMETER: {
                if (path_idx >= path_segment_count) {
                    printf("[DEBUG] route_pattern_match: path too short for parameter segment\n");
                    goto no_match;
                }
                
                char *value = path_segments[path_idx];
                if (!validate_parameter_value(value, seg->param->type)) {
                    printf("[DEBUG] route_pattern_match: parameter validation failed for '%s'\n", value);
                    goto no_match;
                }
                
                // Store parameter
                match.params[param_idx].name = strdup(seg->param->name);
                match.params[param_idx].type = seg->param->type;
                match.params[param_idx].is_optional = seg->param->is_optional;
                match.params[param_idx].value = strdup(value);
                param_idx++;
                path_idx++;
                
                printf("[DEBUG] route_pattern_match: captured parameter '%s' = '%s'\n", 
                       seg->param->name, value);
                break;
            }
            
            case SEGMENT_OPTIONAL: {
                if (path_idx < path_segment_count) {
                    char *value = path_segments[path_idx];
                    if (validate_parameter_value(value, seg->param->type)) {
                        // Optional parameter present and valid
                        match.params[param_idx].name = strdup(seg->param->name);
                        match.params[param_idx].type = seg->param->type;
                        match.params[param_idx].is_optional = seg->param->is_optional;
                        match.params[param_idx].value = strdup(value);
                        param_idx++;
                        path_idx++;
                        
                        printf("[DEBUG] route_pattern_match: captured optional parameter '%s' = '%s'\n", 
                               seg->param->name, value);
                    }
                    // If invalid, treat as not present (optional)
                }
                // Optional parameters can be skipped
                break;
            }
            
            case SEGMENT_WILDCARD: {
                // Capture remaining path as wildcard
                if (path_idx < path_segment_count) {
                    // Build wildcard path from remaining segments
                    int total_len = 0;
                    for (int i = path_idx; i < path_segment_count; i++) {
                        total_len += strlen(path_segments[i]) + 1; // +1 for '/'
                    }
                    
                    match.wildcard_path = malloc(total_len + 1);
                    match.wildcard_path[0] = '\0';
                    
                    for (int i = path_idx; i < path_segment_count; i++) {
                        if (i > path_idx) strcat(match.wildcard_path, "/");
                        strcat(match.wildcard_path, path_segments[i]);
                    }
                    
                    printf("[DEBUG] route_pattern_match: captured wildcard = '%s'\n", match.wildcard_path);
                }
                
                // Wildcard matches everything remaining
                path_idx = path_segment_count;
                break;
            }
        }
    }
    
    // Check if we consumed all path segments (unless there's a wildcard)
    if (path_idx != path_segment_count && !pattern->has_wildcards) {
        printf("[DEBUG] route_pattern_match: path has extra segments\n");
        goto no_match;
    }
    
    match.matched = 1;
    match.param_count = param_idx;
    
    printf("[DEBUG] route_pattern_match: MATCH! captured %d parameters\n", param_idx);
    goto cleanup;
    
no_match:
    printf("[DEBUG] route_pattern_match: NO MATCH\n");
    // Free any allocated memory from partial match
    if (match.params) {
        for (int i = 0; i < match.param_count; i++) {
            if (match.params[i].name) free(match.params[i].name);
            if (match.params[i].value) free(match.params[i].value);
        }
        free(match.params);
        match.params = NULL;
    }
    if (match.wildcard_path) {
        free(match.wildcard_path);
        match.wildcard_path = NULL;
    }
    match.matched = 0;
    match.param_count = 0;
    
cleanup:
    // Free path segments
    for (int i = 0; i < path_segment_count; i++) {
        free(path_segments[i]);
    }
    free(path_segments);
    
    return match;
}

// Free route pattern
void free_route_pattern(RoutePattern *pattern) {
    if (!pattern) return;
    
    free(pattern->original_pattern);
    
    for (int i = 0; i < pattern->segment_count; i++) {
        RouteSegment *seg = &pattern->segments[i];
        
        if (seg->literal_value) {
            free(seg->literal_value);
        }
        
        if (seg->param) {
            free(seg->param->name);
            if (seg->param->value) {
                free(seg->param->value);
            }
            free(seg->param);
        }
    }
    
    free(pattern->segments);
    free(pattern);
}

// Free route match
void free_route_match(RouteMatch *match) {
    if (!match) return;
    
    if (match->params) {
        for (int i = 0; i < match->param_count; i++) {
            if (match->params[i].name) free(match->params[i].name);
            if (match->params[i].value) free(match->params[i].value);
        }
        free(match->params);
    }
    
    if (match->wildcard_path) {
        free(match->wildcard_path);
    }
}

// Helper function to duplicate a RouteMatch structure
RouteMatch* duplicate_route_match(const RouteMatch* original) {
    if (!original || !original->matched) return NULL;
    
    RouteMatch* copy = malloc(sizeof(RouteMatch));
    if (!copy) return NULL;
    
    copy->matched = original->matched;
    copy->param_count = original->param_count;
    
    // Duplicate parameters
    if (original->param_count > 0 && original->params) {
        copy->params = malloc(original->param_count * sizeof(RouteParam));
        if (!copy->params) {
            free(copy);
            return NULL;
        }
        
        for (int i = 0; i < original->param_count; i++) {
            copy->params[i].name = original->params[i].name ? strdup(original->params[i].name) : NULL;
            copy->params[i].value = original->params[i].value ? strdup(original->params[i].value) : NULL;
            copy->params[i].type = original->params[i].type;
            copy->params[i].is_optional = original->params[i].is_optional;
        }
    } else {
        copy->params = NULL;
    }
    
    // Duplicate wildcard path
    copy->wildcard_path = original->wildcard_path ? strdup(original->wildcard_path) : NULL;
    
    return copy;
}

// Constraint creation functions
RouteConstraint* create_min_constraint(long min_value, const char *error_msg) {
    RouteConstraint *constraint = malloc(sizeof(RouteConstraint));
    if (!constraint) return NULL;
    
    constraint->type = CONSTRAINT_MIN;
    constraint->constraint.range.min_value = min_value;
    constraint->constraint.range.max_value = 0; // Not used for MIN
    constraint->error_message = error_msg ? strdup(error_msg) : strdup("Value is too small");
    constraint->context = NULL;
    constraint->next = NULL;
    
    return constraint;
}

RouteConstraint* create_max_constraint(long max_value, const char *error_msg) {
    RouteConstraint *constraint = malloc(sizeof(RouteConstraint));
    if (!constraint) return NULL;
    
    constraint->type = CONSTRAINT_MAX;
    constraint->constraint.range.min_value = 0; // Not used for MAX
    constraint->constraint.range.max_value = max_value;
    constraint->error_message = error_msg ? strdup(error_msg) : strdup("Value is too large");
    constraint->context = NULL;
    constraint->next = NULL;
    
    return constraint;
}

RouteConstraint* create_range_constraint(long min_value, long max_value, const char *error_msg) {
    RouteConstraint *constraint = malloc(sizeof(RouteConstraint));
    if (!constraint) return NULL;
    
    constraint->type = CONSTRAINT_RANGE;
    constraint->constraint.range.min_value = min_value;
    constraint->constraint.range.max_value = max_value;
    constraint->error_message = error_msg ? strdup(error_msg) : strdup("Value is out of range");
    constraint->context = NULL;
    constraint->next = NULL;
    
    return constraint;
}

RouteConstraint* create_regex_constraint(const char *pattern, const char *error_msg) {
    RouteConstraint *constraint = malloc(sizeof(RouteConstraint));
    if (!constraint) return NULL;
    
    constraint->type = CONSTRAINT_REGEX;
    constraint->constraint.regex_pattern = strdup(pattern);
    constraint->error_message = error_msg ? strdup(error_msg) : strdup("Invalid format");
    constraint->context = NULL;
    constraint->next = NULL;
    
    return constraint;
}

RouteConstraint* create_enum_constraint(const char **values, const char *error_msg) {
    RouteConstraint *constraint = malloc(sizeof(RouteConstraint));
    if (!constraint) return NULL;
    
    // Count values
    int count = 0;
    while (values[count]) count++;
    
    // Allocate array and copy values
    constraint->constraint.enum_values = malloc((count + 1) * sizeof(char*));
    for (int i = 0; i < count; i++) {
        constraint->constraint.enum_values[i] = strdup(values[i]);
    }
    constraint->constraint.enum_values[count] = NULL;
    
    constraint->type = CONSTRAINT_ENUM;
    constraint->error_message = error_msg ? strdup(error_msg) : strdup("Invalid value");
    constraint->context = NULL;
    constraint->next = NULL;
    
    return constraint;
}

RouteConstraint* create_custom_constraint(CustomValidator validator, void *context, const char *error_msg) {
    RouteConstraint *constraint = malloc(sizeof(RouteConstraint));
    if (!constraint) return NULL;
    
    constraint->type = CONSTRAINT_CUSTOM;
    constraint->constraint.validator = validator;
    constraint->error_message = error_msg ? strdup(error_msg) : strdup("Validation failed");
    constraint->context = context;
    constraint->next = NULL;
    
    return constraint;
}

// Add constraint to parameter
void add_parameter_constraint(RouteParam *param, RouteConstraint *constraint) {
    if (!param || !constraint) return;
    
    if (!param->constraints) {
        param->constraints = constraint;
    } else {
        // Add to end of chain
        RouteConstraint *current = param->constraints;
        while (current->next) {
            current = current->next;
        }
        current->next = constraint;
    }
}

// Validate parameter against its constraints
int validate_parameter_constraints(const RouteParam *param, ValidationError *error) {
    if (!param || !param->constraints) return 1; // No constraints = valid
    
    RouteConstraint *constraint = param->constraints;
    while (constraint) {
        int valid = 1;
        
        switch (constraint->type) {
            case CONSTRAINT_MIN: {
                if (param->type == PARAM_NUMBER) {
                    long value = strtol(param->value, NULL, 10);
                    valid = (value >= constraint->constraint.range.min_value);
                } else {
                    // For strings, check length
                    valid = (strlen(param->value) >= (size_t)constraint->constraint.range.min_value);
                }
                break;
            }
            
            case CONSTRAINT_MAX: {
                if (param->type == PARAM_NUMBER) {
                    long value = strtol(param->value, NULL, 10);
                    valid = (value <= constraint->constraint.range.max_value);
                } else {
                    // For strings, check length
                    valid = (strlen(param->value) <= (size_t)constraint->constraint.range.max_value);
                }
                break;
            }
            
            case CONSTRAINT_RANGE: {
                if (param->type == PARAM_NUMBER) {
                    long value = strtol(param->value, NULL, 10);
                    valid = (value >= constraint->constraint.range.min_value && 
                            value <= constraint->constraint.range.max_value);
                } else {
                    // For strings, check length
                    size_t len = strlen(param->value);
                    valid = (len >= (size_t)constraint->constraint.range.min_value && 
                            len <= (size_t)constraint->constraint.range.max_value);
                }
                break;
            }
            
            case CONSTRAINT_REGEX: {
                regex_t regex;
                int ret = regcomp(&regex, constraint->constraint.regex_pattern, REG_EXTENDED);
                if (ret == 0) {
                    ret = regexec(&regex, param->value, 0, NULL, 0);
                    valid = (ret == 0);
                    regfree(&regex);
                } else {
                    valid = 0; // Invalid regex = fail
                }
                break;
            }
            
            case CONSTRAINT_ENUM: {
                valid = 0;
                for (int i = 0; constraint->constraint.enum_values[i]; i++) {
                    if (strcmp(param->value, constraint->constraint.enum_values[i]) == 0) {
                        valid = 1;
                        break;
                    }
                }
                break;
            }
            
            case CONSTRAINT_CUSTOM: {
                valid = constraint->constraint.validator(param->value, constraint->context, error);
                break;
            }
        }
        
        if (!valid) {
            // Fill error information
            if (error) {
                error->param_name = strdup(param->name);
                error->provided_value = strdup(param->value);
                error->failed_constraint = constraint->type;
                if (!error->error_message) { // Custom validator might set this
                    error->error_message = strdup(constraint->error_message);
                }
            }
            return 0;
        }
        
        constraint = constraint->next;
    }
    
    return 1;
}

// Validate all parameters in a route match
int validate_route_match(const RouteMatch *match, ValidationError **errors, int *error_count) {
    if (!match || !match->matched) return 1;
    
    *errors = NULL;
    *error_count = 0;
    
    if (match->param_count == 0) return 1;
    
    // Allocate array for potential errors
    ValidationError *error_array = malloc(match->param_count * sizeof(ValidationError));
    int actual_errors = 0;
    
    for (int i = 0; i < match->param_count; i++) {
        ValidationError error = {0};
        if (!validate_parameter_constraints(&match->params[i], &error)) {
            error_array[actual_errors] = error;
            actual_errors++;
        }
    }
    
    if (actual_errors > 0) {
        *errors = realloc(error_array, actual_errors * sizeof(ValidationError));
        *error_count = actual_errors;
        return 0;
    } else {
        free(error_array);
        return 1;
    }
}

// Free constraint
void free_constraint(RouteConstraint *constraint) {
    if (!constraint) return;
    
    if (constraint->next) {
        free_constraint(constraint->next);
    }
    
    switch (constraint->type) {
        case CONSTRAINT_REGEX:
            free(constraint->constraint.regex_pattern);
            break;
        case CONSTRAINT_ENUM:
            for (int i = 0; constraint->constraint.enum_values[i]; i++) {
                free(constraint->constraint.enum_values[i]);
            }
            free(constraint->constraint.enum_values);
            break;
        default:
            // Other types don't have allocated memory
            break;
    }
    
    free(constraint->error_message);
    free(constraint);
}

// Free validation error
void free_validation_error(ValidationError *error) {
    if (!error) return;
    
    free(error->param_name);
    free(error->error_message);
    free(error->provided_value);
}
