#include "../src/parsers/json.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== Complex JSON Memory Test ===\n\n");
    
    // Test complex nested JSON that would trigger multiple parse_string calls
    const char *complex_json = "{"
        "\"users\": ["
        "  {\"name\": \"Alice\", \"email\": \"alice@example.com\", \"settings\": {\"theme\": \"dark\"}},"
        "  {\"name\": \"Bob\", \"email\": \"bob@example.com\", \"settings\": {\"theme\": \"light\"}}"
        "],"
        "\"metadata\": {"
        "  \"version\": \"1.0.0\","
        "  \"description\": \"Test data for complex JSON parsing\""
        "}"
        "}";
    
    printf("Testing complex JSON with nested objects and arrays...\n");
    printf("Input length: %zu characters\n", strlen(complex_json));
    
    JsonValue *value = json_parse(complex_json);
    if (!value) {
        printf("Failed to parse complex JSON\n");
        return 1;
    }
    
    printf("Successfully parsed complex JSON\n");
    printf("Type: %s\n", json_type_name(value->type));
    
    if (value->type == JSON_OBJECT) {
        JsonObject *obj = value->data.object_value;
        printf("Object has %d properties\n", obj->property_count);
        
        // Test accessing nested data
        JsonValue *users = json_object_get(obj, "users");
        if (users && users->type == JSON_ARRAY) {
            printf("Found users array with %d items\n", json_array_size(users->data.array_value));
        }
        
        JsonValue *metadata = json_object_get(obj, "metadata");
        if (metadata && metadata->type == JSON_OBJECT) {
            const char *version = json_object_get_string(metadata->data.object_value, "version");
            printf("Version: %s\n", version ? version : "null");
        }
    }
    
    // Free the JSON value (this will test our memory management)
    json_free_value(value);
    printf("JSON memory freed successfully\n");
    
    // Test multiple parse/free cycles
    printf("\nTesting multiple parse/free cycles...\n");
    for (int i = 0; i < 5; i++) {
        JsonValue *test_value = json_parse(complex_json);
        if (test_value) {
            json_free_value(test_value);
            printf("Cycle %d: [OK]\n", i + 1);
        } else {
            printf("Cycle %d: [FAIL]\n", i + 1);
            return 1;
        }
    }
    
    printf("\n=== All Memory Tests Passed! ===\n");
    return 0;
}
