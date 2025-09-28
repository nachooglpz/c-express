#include "json.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== Simple JSON Parser Test ===\n\n");
    
    // Test 1: Simple object
    const char *json1 = "{\"name\":\"Alice\",\"age\":25,\"active\":true}";
    printf("Test 1: Parsing simple object\n");
    printf("Input: %s\n", json1);
    
    char *error = NULL;
    JsonValue *value1 = json_parse_with_error(json1, &error);
    
    if (error) {
        printf("Error: %s\n", error);
        free(error);
    } else if (value1) {
        printf("Success! Type: %s\n", json_type_name(value1->type));
        
        if (value1->type == JSON_OBJECT) {
            JsonObject *obj = value1->data.object_value;
            const char *name = json_object_get_string(obj, "name");
            double age = json_object_get_number(obj, "age");
            int active = json_object_get_bool(obj, "active");
            
            printf("  name: %s\n", name ? name : "null");
            printf("  age: %.0f\n", age);
            printf("  active: %s\n", active ? "true" : "false");
        }
        
        json_free_value(value1);
    }
    
    printf("\n");
    
    // Test 2: Array
    const char *json2 = "[{\"id\":1},{\"id\":2}]";
    printf("Test 2: Parsing array\n");
    printf("Input: %s\n", json2);
    
    JsonValue *value2 = json_parse_with_error(json2, &error);
    
    if (error) {
        printf("Error: %s\n", error);
        free(error);
    } else if (value2) {
        printf("Success! Type: %s\n", json_type_name(value2->type));
        
        if (value2->type == JSON_ARRAY) {
            JsonArray *arr = value2->data.array_value;
            printf("  Array size: %d\n", json_array_size(arr));
        }
        
        json_free_value(value2);
    }
    
    printf("\n");
    
    // Test 3: Schema validation
    printf("Test 3: Schema validation\n");
    const char *json3 = "{\"name\":\"Bob\",\"email\":\"bob@test.com\",\"age\":30}";
    printf("Input: %s\n", json3);
    
    JsonValue *value3 = json_parse(json3);
    if (value3) {
        JsonSchema *schema = json_create_schema("user");
        json_schema_add_string_field(schema, "name", JSON_FIELD_REQUIRED, 2, 50);
        json_schema_add_string_field(schema, "email", JSON_FIELD_REQUIRED, 5, 100);
        json_schema_add_number_field(schema, "age", JSON_FIELD_OPTIONAL, 0, 150);
        
        JsonValidationResult *result = json_validate_schema(value3, schema);
        if (result) {
            printf("  Validation: %s\n", result->is_valid ? "PASSED" : "FAILED");
            
            if (!result->is_valid) {
                for (int i = 0; i < result->error_count; i++) {
                    printf("    Error: %s - %s\n", 
                           result->errors[i].field_path,
                           result->errors[i].error_message);
                }
            }
            
            json_free_validation_result(result);
        }
        
        json_free_schema(schema);
        json_free_value(value3);
    }
    
    printf("\n=== JSON Parser Test Complete ===\n");
    return 0;
}
