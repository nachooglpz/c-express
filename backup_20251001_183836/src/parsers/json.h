#ifndef JSON_H
#define JSON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// JSON value types
typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonType;

// Forward declarations
typedef struct JsonValue JsonValue;
typedef struct JsonObject JsonObject;
typedef struct JsonArray JsonArray;

// JSON value structure
struct JsonValue {
    JsonType type;
    union {
        int bool_value;
        double number_value;
        char *string_value;
        JsonObject *object_value;
        JsonArray *array_value;
    } data;
};

// JSON object (key-value pairs)
typedef struct JsonProperty {
    char *key;
    JsonValue *value;
    struct JsonProperty *next;
} JsonProperty;

struct JsonObject {
    JsonProperty *properties;
    int property_count;
};

// JSON array
struct JsonArray {
    JsonValue **items;
    int count;
    int capacity;
};

// JSON parsing context
typedef struct {
    const char *json_text;
    size_t position;
    size_t length;
    char *error_message;
} JsonParser;

// JSON validation schema
typedef enum {
    JSON_FIELD_REQUIRED = 1,
    JSON_FIELD_OPTIONAL = 0
} JsonFieldRequired;

typedef struct {
    char *field_name;
    JsonType expected_type;
    JsonFieldRequired required;
    char *validation_pattern;    // For string validation (regex)
    double min_value;           // For number validation
    double max_value;           // For number validation
    int min_length;             // For string/array length validation
    int max_length;             // For string/array length validation
} JsonFieldValidator;

typedef struct {
    JsonFieldValidator *validators;
    int validator_count;
    char *schema_name;
} JsonSchema;

// JSON parsing functions
JsonValue* json_parse(const char *json_text);
JsonValue* json_parse_with_error(const char *json_text, char **error_message);
void json_free_value(JsonValue *value);

// JSON object functions
JsonObject* json_create_object(void);
JsonValue* json_object_get(JsonObject *obj, const char *key);
const char* json_object_get_string(JsonObject *obj, const char *key);
double json_object_get_number(JsonObject *obj, const char *key);
int json_object_get_bool(JsonObject *obj, const char *key);
JsonObject* json_object_get_object(JsonObject *obj, const char *key);
JsonArray* json_object_get_array(JsonObject *obj, const char *key);
void json_object_set(JsonObject *obj, const char *key, JsonValue *value);
int json_object_has_key(JsonObject *obj, const char *key);

// JSON array functions
JsonArray* json_create_array(void);
JsonValue* json_array_get(JsonArray *arr, int index);
void json_array_add(JsonArray *arr, JsonValue *value);
int json_array_size(JsonArray *arr);

// JSON value creation functions
JsonValue* json_create_null(void);
JsonValue* json_create_bool(int value);
JsonValue* json_create_number(double value);
JsonValue* json_create_string(const char *value);
JsonValue* json_create_object_value(JsonObject *obj);
JsonValue* json_create_array_value(JsonArray *arr);

// JSON serialization
char* json_serialize(JsonValue *value);
char* json_serialize_pretty(JsonValue *value, int indent);

// JSON validation
typedef struct {
    char *field_path;
    char *error_message;
    JsonType expected_type;
    JsonType actual_type;
} JsonValidationError;

typedef struct {
    JsonValidationError *errors;
    int error_count;
    int is_valid;
} JsonValidationResult;

JsonValidationResult* json_validate_schema(JsonValue *json, JsonSchema *schema);
void json_free_validation_result(JsonValidationResult *result);

// JSON schema creation helpers
JsonSchema* json_create_schema(const char *schema_name);
void json_schema_add_field(JsonSchema *schema, const char *field_name, 
                          JsonType type, JsonFieldRequired required);
void json_schema_add_string_field(JsonSchema *schema, const char *field_name,
                                 JsonFieldRequired required, int min_len, int max_len);
void json_schema_add_number_field(JsonSchema *schema, const char *field_name,
                                 JsonFieldRequired required, double min_val, double max_val);
void json_free_schema(JsonSchema *schema);

// Utility functions
const char* json_type_name(JsonType type);
void json_print_value(JsonValue *value, int indent);

#endif
