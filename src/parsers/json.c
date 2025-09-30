#define _GNU_SOURCE
#include "json.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <limits.h>

// ============================================================================
// JSON PARSER IMPLEMENTATION
// ============================================================================

// Helper functions for parsing
static void skip_whitespace(JsonParser *parser) {
    while (parser->position < parser->length) {
        char c = parser->json_text[parser->position];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            parser->position++;
        } else {
            break;
        }
    }
}

static char peek_char(JsonParser *parser) {
    if (parser->position >= parser->length) return '\0';
    return parser->json_text[parser->position];
}

static char next_char(JsonParser *parser) {
    if (parser->position >= parser->length) return '\0';
    return parser->json_text[parser->position++];
}

static void set_error(JsonParser *parser, const char *message) {
    if (parser->error_message) return; // Already has error
    parser->error_message = strdup(message);
}

// Forward declarations for recursive parsing
static JsonValue* parse_value(JsonParser *parser);
static JsonObject* parse_object(JsonParser *parser);
static JsonArray* parse_array(JsonParser *parser);
static char* parse_string(JsonParser *parser);
static double parse_number(JsonParser *parser);

// Parse JSON string with escape sequences
static char* parse_string(JsonParser *parser) {
    if (next_char(parser) != '"') {
        set_error(parser, "Expected '\"' at start of string");
        return NULL;
    }
    
    size_t start = parser->position;
    (void)start; // Suppress unused variable warning
    size_t str_length = 0;
    
    // First pass: calculate length and validate
    size_t pos = parser->position;
    while (pos < parser->length) {
        char c = parser->json_text[pos];
        if (c == '"') {
            break;
        } else if (c == '\\') {
            pos++; // Skip escape character
            if (pos >= parser->length) {
                set_error(parser, "Unterminated string escape");
                return NULL;
            }
            char escaped = parser->json_text[pos];
            switch (escaped) {
                case '"': case '\\': case '/': case 'b': 
                case 'f': case 'n': case 'r': case 't':
                    str_length++;
                    break;
                case 'u':
                    // Unicode escape \uXXXX - would need 4 hex digits
                    str_length += 4; // Simplified - would convert to UTF-8
                    pos += 4;
                    break;
                default:
                    set_error(parser, "Invalid escape sequence");
                    return NULL;
            }
        } else if (c < 0x20) {
            set_error(parser, "Control character in string");
            return NULL;
        } else {
            str_length++;
        }
        pos++;
    }
    
    if (pos >= parser->length) {
        set_error(parser, "Unterminated string");
        return NULL;
    }
    
    // Second pass: build string
    char *result = malloc(str_length + 1);
    if (!result) {
        set_error(parser, "Memory allocation failed");
        return NULL;
    }
    
    size_t result_pos = 0;
    while (parser->position < parser->length) {
        char c = next_char(parser);
        if (c == '"') {
            result[result_pos] = '\0';
            return result;
        } else if (c == '\\') {
            char escaped = next_char(parser);
            switch (escaped) {
                case '"': result[result_pos++] = '"'; break;
                case '\\': result[result_pos++] = '\\'; break;
                case '/': result[result_pos++] = '/'; break;
                case 'b': result[result_pos++] = '\b'; break;
                case 'f': result[result_pos++] = '\f'; break;
                case 'n': result[result_pos++] = '\n'; break;
                case 'r': result[result_pos++] = '\r'; break;
                case 't': result[result_pos++] = '\t'; break;
                case 'u':
                    // Simplified Unicode handling - just copy the sequence
                    result[result_pos++] = '\\';
                    result[result_pos++] = 'u';
                    for (int i = 0; i < 4; i++) {
                        result[result_pos++] = next_char(parser);
                    }
                    break;
            }
        } else {
            result[result_pos++] = c;
        }
    }
    
    free(result);
    set_error(parser, "Unterminated string");
    return NULL;
}

// Parse JSON number
static double parse_number(JsonParser *parser) {
    size_t start = parser->position;
    
    // Handle optional minus
    if (peek_char(parser) == '-') {
        next_char(parser);
    }
    
    // Parse integer part
    if (peek_char(parser) == '0') {
        next_char(parser);
    } else if (isdigit(peek_char(parser))) {
        while (isdigit(peek_char(parser))) {
            next_char(parser);
        }
    } else {
        set_error(parser, "Invalid number format");
        return 0.0;
    }
    
    // Parse fractional part
    if (peek_char(parser) == '.') {
        next_char(parser);
        if (!isdigit(peek_char(parser))) {
            set_error(parser, "Expected digit after decimal point");
            return 0.0;
        }
        while (isdigit(peek_char(parser))) {
            next_char(parser);
        }
    }
    
    // Parse exponent
    char c = peek_char(parser);
    if (c == 'e' || c == 'E') {
        next_char(parser);
        c = peek_char(parser);
        if (c == '+' || c == '-') {
            next_char(parser);
        }
        if (!isdigit(peek_char(parser))) {
            set_error(parser, "Expected digit in exponent");
            return 0.0;
        }
        while (isdigit(peek_char(parser))) {
            next_char(parser);
        }
    }
    
    // Extract number string and convert
    size_t length = parser->position - start;
    char *number_str = malloc(length + 1);
    if (!number_str) {
        set_error(parser, "Memory allocation failed");
        return 0.0;
    }
    
    strncpy(number_str, parser->json_text + start, length);
    number_str[length] = '\0';
    
    char *endptr;
    errno = 0;
    double result = strtod(number_str, &endptr);
    
    if (errno == ERANGE || endptr != number_str + length) {
        free(number_str);
        set_error(parser, "Invalid number value");
        return 0.0;
    }
    
    free(number_str);
    return result;
}

// Parse JSON array
static JsonArray* parse_array(JsonParser *parser) {
    if (next_char(parser) != '[') {
        set_error(parser, "Expected '[' at start of array");
        return NULL;
    }
    
    JsonArray *array = json_create_array();
    if (!array) {
        set_error(parser, "Memory allocation failed");
        return NULL;
    }
    
    skip_whitespace(parser);
    
    // Handle empty array
    if (peek_char(parser) == ']') {
        next_char(parser);
        return array;
    }
    
    // Parse array elements
    while (parser->position < parser->length && !parser->error_message) {
        JsonValue *value = parse_value(parser);
        if (!value) {
            json_free_value(json_create_array_value(array));
            return NULL;
        }
        
        json_array_add(array, value);
        
        skip_whitespace(parser);
        char c = peek_char(parser);
        
        if (c == ']') {
            next_char(parser);
            return array;
        } else if (c == ',') {
            next_char(parser);
            skip_whitespace(parser);
        } else {
            set_error(parser, "Expected ',' or ']' in array");
            json_free_value(json_create_array_value(array));
            return NULL;
        }
    }
    
    set_error(parser, "Unterminated array");
    json_free_value(json_create_array_value(array));
    return NULL;
}

// Parse JSON object
static JsonObject* parse_object(JsonParser *parser) {
    if (next_char(parser) != '{') {
        set_error(parser, "Expected '{' at start of object");
        return NULL;
    }
    
    JsonObject *object = json_create_object();
    if (!object) {
        set_error(parser, "Memory allocation failed");
        return NULL;
    }
    
    skip_whitespace(parser);
    
    // Handle empty object
    if (peek_char(parser) == '}') {
        next_char(parser);
        return object;
    }
    
    // Parse object properties
    while (parser->position < parser->length && !parser->error_message) {
        skip_whitespace(parser);
        
        // Parse key
        if (peek_char(parser) != '"') {
            set_error(parser, "Expected string key in object");
            json_free_value(json_create_object_value(object));
            return NULL;
        }
        
        char *key = parse_string(parser);
        if (!key) {
            json_free_value(json_create_object_value(object));
            return NULL;
        }
        
        skip_whitespace(parser);
        
        // Parse colon
        if (next_char(parser) != ':') {
            set_error(parser, "Expected ':' after object key");
            free(key);
            json_free_value(json_create_object_value(object));
            return NULL;
        }
        
        skip_whitespace(parser);
        
        // Parse value
        JsonValue *value = parse_value(parser);
        if (!value) {
            free(key);
            json_free_value(json_create_object_value(object));
            return NULL;
        }
        
        json_object_set(object, key, value);
        free(key);
        
        skip_whitespace(parser);
        char c = peek_char(parser);
        
        if (c == '}') {
            next_char(parser);
            return object;
        } else if (c == ',') {
            next_char(parser);
            skip_whitespace(parser);
        } else {
            set_error(parser, "Expected ',' or '}' in object");
            json_free_value(json_create_object_value(object));
            return NULL;
        }
    }
    
    set_error(parser, "Unterminated object");
    json_free_value(json_create_object_value(object));
    return NULL;
}

// Parse JSON value (recursive)
static JsonValue* parse_value(JsonParser *parser) {
    skip_whitespace(parser);
    
    char c = peek_char(parser);
    
    if (c == '"') {
        // String
        char *str = parse_string(parser);
        return str ? json_create_string(str) : NULL;
    } else if (c == '{') {
        // Object
        JsonObject *obj = parse_object(parser);
        return obj ? json_create_object_value(obj) : NULL;
    } else if (c == '[') {
        // Array
        JsonArray *arr = parse_array(parser);
        return arr ? json_create_array_value(arr) : NULL;
    } else if (c == 't') {
        // true
        if (parser->position + 4 <= parser->length && 
            strncmp(parser->json_text + parser->position, "true", 4) == 0) {
            parser->position += 4;
            return json_create_bool(1);
        } else {
            set_error(parser, "Invalid literal 'true'");
            return NULL;
        }
    } else if (c == 'f') {
        // false
        if (parser->position + 5 <= parser->length && 
            strncmp(parser->json_text + parser->position, "false", 5) == 0) {
            parser->position += 5;
            return json_create_bool(0);
        } else {
            set_error(parser, "Invalid literal 'false'");
            return NULL;
        }
    } else if (c == 'n') {
        // null
        if (parser->position + 4 <= parser->length && 
            strncmp(parser->json_text + parser->position, "null", 4) == 0) {
            parser->position += 4;
            return json_create_null();
        } else {
            set_error(parser, "Invalid literal 'null'");
            return NULL;
        }
    } else if (c == '-' || isdigit(c)) {
        // Number
        double num = parse_number(parser);
        return parser->error_message ? NULL : json_create_number(num);
    } else {
        set_error(parser, "Unexpected character");
        return NULL;
    }
}

// Main parsing functions
JsonValue* json_parse(const char *json_text) {
    char *error = NULL;
    JsonValue *result = json_parse_with_error(json_text, &error);
    if (error) {
        printf("[JSON Parse Error]: %s\n", error);
        free(error);
    }
    return result;
}

JsonValue* json_parse_with_error(const char *json_text, char **error_message) {
    if (!json_text) {
        *error_message = strdup("JSON text is NULL");
        return NULL;
    }
    
    JsonParser parser = {
        .json_text = json_text,
        .position = 0,
        .length = strlen(json_text),
        .error_message = NULL
    };
    
    JsonValue *result = parse_value(&parser);
    
    if (parser.error_message) {
        *error_message = parser.error_message;
        if (result) {
            json_free_value(result);
            result = NULL;
        }
    } else {
        *error_message = NULL;
        
        // Check for trailing content
        skip_whitespace(&parser);
        if (parser.position < parser.length) {
            *error_message = strdup("Unexpected content after JSON value");
            if (result) {
                json_free_value(result);
                result = NULL;
            }
        }
    }
    
    return result;
}

// ============================================================================
// JSON VALUE CREATION AND MANAGEMENT
// ============================================================================

// Create JSON values
JsonValue* json_create_null(void) {
    JsonValue *value = malloc(sizeof(JsonValue));
    if (!value) return NULL;
    value->type = JSON_NULL;
    return value;
}

JsonValue* json_create_bool(int bool_val) {
    JsonValue *value = malloc(sizeof(JsonValue));
    if (!value) return NULL;
    value->type = JSON_BOOL;
    value->data.bool_value = bool_val ? 1 : 0;
    return value;
}

JsonValue* json_create_number(double number) {
    JsonValue *value = malloc(sizeof(JsonValue));
    if (!value) return NULL;
    value->type = JSON_NUMBER;
    value->data.number_value = number;
    return value;
}

JsonValue* json_create_string(const char *string) {
    if (!string) return json_create_null();
    
    JsonValue *value = malloc(sizeof(JsonValue));
    if (!value) return NULL;
    
    value->type = JSON_STRING;
    value->data.string_value = strdup(string);
    if (!value->data.string_value) {
        free(value);
        return NULL;
    }
    return value;
}

JsonValue* json_create_object_value(JsonObject *object) {
    JsonValue *value = malloc(sizeof(JsonValue));
    if (!value) return NULL;
    value->type = JSON_OBJECT;
    value->data.object_value = object;
    return value;
}

JsonValue* json_create_array_value(JsonArray *array) {
    JsonValue *value = malloc(sizeof(JsonValue));
    if (!value) return NULL;
    value->type = JSON_ARRAY;
    value->data.array_value = array;
    return value;
}

// Free JSON values
void json_free_value(JsonValue *value) {
    if (!value) return;
    
    switch (value->type) {
        case JSON_STRING:
            free(value->data.string_value);
            break;
        case JSON_OBJECT:
            if (value->data.object_value) {
                JsonProperty *prop = value->data.object_value->properties;
                while (prop) {
                    JsonProperty *next = prop->next;
                    free(prop->key);
                    json_free_value(prop->value);
                    free(prop);
                    prop = next;
                }
                free(value->data.object_value);
            }
            break;
        case JSON_ARRAY:
            if (value->data.array_value) {
                for (int i = 0; i < value->data.array_value->count; i++) {
                    json_free_value(value->data.array_value->items[i]);
                }
                free(value->data.array_value->items);
                free(value->data.array_value);
            }
            break;
        case JSON_NULL:
        case JSON_BOOL:
        case JSON_NUMBER:
            // No additional cleanup needed
            break;
    }
    
    free(value);
}

// ============================================================================
// JSON OBJECT FUNCTIONS
// ============================================================================

JsonObject* json_create_object(void) {
    JsonObject *obj = malloc(sizeof(JsonObject));
    if (!obj) return NULL;
    
    obj->properties = NULL;
    obj->property_count = 0;
    return obj;
}

JsonValue* json_object_get(JsonObject *obj, const char *key) {
    if (!obj || !key) return NULL;
    
    JsonProperty *prop = obj->properties;
    while (prop) {
        if (strcmp(prop->key, key) == 0) {
            return prop->value;
        }
        prop = prop->next;
    }
    return NULL;
}

const char* json_object_get_string(JsonObject *obj, const char *key) {
    JsonValue *value = json_object_get(obj, key);
    if (value && value->type == JSON_STRING) {
        return value->data.string_value;
    }
    return NULL;
}

double json_object_get_number(JsonObject *obj, const char *key) {
    JsonValue *value = json_object_get(obj, key);
    if (value && value->type == JSON_NUMBER) {
        return value->data.number_value;
    }
    return 0.0;
}

int json_object_get_bool(JsonObject *obj, const char *key) {
    JsonValue *value = json_object_get(obj, key);
    if (value && value->type == JSON_BOOL) {
        return value->data.bool_value;
    }
    return 0;
}

JsonObject* json_object_get_object(JsonObject *obj, const char *key) {
    JsonValue *value = json_object_get(obj, key);
    if (value && value->type == JSON_OBJECT) {
        return value->data.object_value;
    }
    return NULL;
}

JsonArray* json_object_get_array(JsonObject *obj, const char *key) {
    JsonValue *value = json_object_get(obj, key);
    if (value && value->type == JSON_ARRAY) {
        return value->data.array_value;
    }
    return NULL;
}

void json_object_set(JsonObject *obj, const char *key, JsonValue *value) {
    if (!obj || !key || !value) return;
    
    // Check if key already exists
    JsonProperty *prop = obj->properties;
    while (prop) {
        if (strcmp(prop->key, key) == 0) {
            // Update existing property
            json_free_value(prop->value);
            prop->value = value;
            return;
        }
        prop = prop->next;
    }
    
    // Add new property
    JsonProperty *new_prop = malloc(sizeof(JsonProperty));
    if (!new_prop) return;
    
    new_prop->key = strdup(key);
    new_prop->value = value;
    new_prop->next = obj->properties;
    obj->properties = new_prop;
    obj->property_count++;
}

int json_object_has_key(JsonObject *obj, const char *key) {
    return json_object_get(obj, key) != NULL;
}

// ============================================================================
// JSON ARRAY FUNCTIONS  
// ============================================================================

JsonArray* json_create_array(void) {
    JsonArray *arr = malloc(sizeof(JsonArray));
    if (!arr) return NULL;
    
    arr->items = NULL;
    arr->count = 0;
    arr->capacity = 0;
    return arr;
}

JsonValue* json_array_get(JsonArray *arr, int index) {
    if (!arr || index < 0 || index >= arr->count) return NULL;
    return arr->items[index];
}

void json_array_add(JsonArray *arr, JsonValue *value) {
    if (!arr || !value) return;
    
    if (arr->count >= arr->capacity) {
        int new_capacity = arr->capacity == 0 ? 4 : arr->capacity * 2;
        JsonValue **new_items = realloc(arr->items, new_capacity * sizeof(JsonValue*));
        if (!new_items) return;
        
        arr->items = new_items;
        arr->capacity = new_capacity;
    }
    
    arr->items[arr->count++] = value;
}

int json_array_size(JsonArray *arr) {
    return arr ? arr->count : 0;
}

// ============================================================================
// JSON VALIDATION AND SCHEMA
// ============================================================================

JsonSchema* json_create_schema(const char *schema_name) {
    JsonSchema *schema = malloc(sizeof(JsonSchema));
    if (!schema) return NULL;
    
    schema->validators = NULL;
    schema->validator_count = 0;
    schema->schema_name = schema_name ? strdup(schema_name) : NULL;
    return schema;
}

void json_schema_add_field(JsonSchema *schema, const char *field_name, 
                          JsonType type, JsonFieldRequired required) {
    if (!schema || !field_name) return;
    
    schema->validators = realloc(schema->validators, 
                               sizeof(JsonFieldValidator) * (schema->validator_count + 1));
    if (!schema->validators) return;
    
    JsonFieldValidator *validator = &schema->validators[schema->validator_count];
    validator->field_name = strdup(field_name);
    validator->expected_type = type;
    validator->required = required;
    validator->validation_pattern = NULL;
    validator->min_value = -INFINITY;
    validator->max_value = INFINITY;
    validator->min_length = 0;
    validator->max_length = INT_MAX;
    
    schema->validator_count++;
}

void json_schema_add_string_field(JsonSchema *schema, const char *field_name,
                                 JsonFieldRequired required, int min_len, int max_len) {
    json_schema_add_field(schema, field_name, JSON_STRING, required);
    
    if (schema->validator_count > 0) {
        JsonFieldValidator *validator = &schema->validators[schema->validator_count - 1];
        validator->min_length = min_len;
        validator->max_length = max_len;
    }
}

void json_schema_add_number_field(JsonSchema *schema, const char *field_name,
                                 JsonFieldRequired required, double min_val, double max_val) {
    json_schema_add_field(schema, field_name, JSON_NUMBER, required);
    
    if (schema->validator_count > 0) {
        JsonFieldValidator *validator = &schema->validators[schema->validator_count - 1];
        validator->min_value = min_val;
        validator->max_value = max_val;
    }
}

JsonValidationResult* json_validate_schema(JsonValue *json, JsonSchema *schema) {
    JsonValidationResult *result = malloc(sizeof(JsonValidationResult));
    if (!result) return NULL;
    
    result->errors = NULL;
    result->error_count = 0;
    result->is_valid = 1;
    
    if (!json || json->type != JSON_OBJECT || !schema) {
        result->is_valid = 0;
        return result;
    }
    
    JsonObject *obj = json->data.object_value;
    
    // Validate each field in schema
    for (int i = 0; i < schema->validator_count; i++) {
        JsonFieldValidator *validator = &schema->validators[i];
        JsonValue *field_value = json_object_get(obj, validator->field_name);
        
        // Check if required field is missing
        if (validator->required == JSON_FIELD_REQUIRED && !field_value) {
            result->is_valid = 0;
            result->errors = realloc(result->errors, 
                                   sizeof(JsonValidationError) * (result->error_count + 1));
            
            JsonValidationError *error = &result->errors[result->error_count];
            error->field_path = strdup(validator->field_name);
            error->error_message = strdup("Required field is missing");
            error->expected_type = validator->expected_type;
            error->actual_type = JSON_NULL;
            result->error_count++;
            continue;
        }
        
        // Skip optional missing fields
        if (!field_value) continue;
        
        // Check type mismatch
        if (field_value->type != validator->expected_type) {
            result->is_valid = 0;
            result->errors = realloc(result->errors, 
                                   sizeof(JsonValidationError) * (result->error_count + 1));
            
            JsonValidationError *error = &result->errors[result->error_count];
            error->field_path = strdup(validator->field_name);
            error->error_message = strdup("Type mismatch");
            error->expected_type = validator->expected_type;
            error->actual_type = field_value->type;
            result->error_count++;
            continue;
        }
        
        // Type-specific validation
        if (validator->expected_type == JSON_STRING) {
            const char *str_val = field_value->data.string_value;
            int len = str_val ? strlen(str_val) : 0;
            
            if (len < validator->min_length || len > validator->max_length) {
                result->is_valid = 0;
                result->errors = realloc(result->errors, 
                                       sizeof(JsonValidationError) * (result->error_count + 1));
                
                JsonValidationError *error = &result->errors[result->error_count];
                error->field_path = strdup(validator->field_name);
                error->error_message = strdup("String length out of bounds");
                error->expected_type = validator->expected_type;
                error->actual_type = field_value->type;
                result->error_count++;
            }
        } else if (validator->expected_type == JSON_NUMBER) {
            double num_val = field_value->data.number_value;
            
            if (num_val < validator->min_value || num_val > validator->max_value) {
                result->is_valid = 0;
                result->errors = realloc(result->errors, 
                                       sizeof(JsonValidationError) * (result->error_count + 1));
                
                JsonValidationError *error = &result->errors[result->error_count];
                error->field_path = strdup(validator->field_name);
                error->error_message = strdup("Number value out of bounds");
                error->expected_type = validator->expected_type;
                error->actual_type = field_value->type;
                result->error_count++;
            }
        }
    }
    
    return result;
}

void json_free_validation_result(JsonValidationResult *result) {
    if (!result) return;
    
    if (result->errors) {
        for (int i = 0; i < result->error_count; i++) {
            free(result->errors[i].field_path);
            free(result->errors[i].error_message);
        }
        free(result->errors);
    }
    
    free(result);
}

void json_free_schema(JsonSchema *schema) {
    if (!schema) return;
    
    if (schema->validators) {
        for (int i = 0; i < schema->validator_count; i++) {
            free(schema->validators[i].field_name);
            free(schema->validators[i].validation_pattern);
        }
        free(schema->validators);
    }
    
    free(schema->schema_name);
    free(schema);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

const char* json_type_name(JsonType type) {
    switch (type) {
        case JSON_NULL: return "null";
        case JSON_BOOL: return "boolean";
        case JSON_NUMBER: return "number";
        case JSON_STRING: return "string";
        case JSON_ARRAY: return "array";
        case JSON_OBJECT: return "object";
        default: return "unknown";
    }
}

void json_print_value(JsonValue *value, int indent) {
    if (!value) {
        printf("null");
        return;
    }
    
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch (value->type) {
        case JSON_NULL:
            printf("null");
            break;
        case JSON_BOOL:
            printf("%s", value->data.bool_value ? "true" : "false");
            break;
        case JSON_NUMBER:
            printf("%g", value->data.number_value);
            break;
        case JSON_STRING:
            printf("\"%s\"", value->data.string_value ? value->data.string_value : "");
            break;
        case JSON_ARRAY: {
            JsonArray *arr = value->data.array_value;
            printf("[\n");
            for (int i = 0; i < arr->count; i++) {
                json_print_value(arr->items[i], indent + 1);
                if (i < arr->count - 1) printf(",");
                printf("\n");
            }
            for (int i = 0; i < indent; i++) printf("  ");
            printf("]");
            break;
        }
        case JSON_OBJECT: {
            JsonObject *obj = value->data.object_value;
            printf("{\n");
            JsonProperty *prop = obj->properties;
            int count = 0;
            while (prop) {
                for (int i = 0; i <= indent; i++) printf("  ");
                printf("\"%s\": ", prop->key);
                json_print_value(prop->value, 0);
                count++;
                if (count < obj->property_count) printf(",");
                printf("\n");
                prop = prop->next;
            }
            for (int i = 0; i < indent; i++) printf("  ");
            printf("}");
            break;
        }
    }
}
