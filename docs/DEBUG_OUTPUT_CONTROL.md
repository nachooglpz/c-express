## Debug and Error Output Documentation

### Debug Output Control
The framework uses conditional compilation to control debug output:

```c
// Environment variable controls debug output
C_EXPRESS_DEBUG=1 ./your-app    // Shows debug output
./your-app                      // Silent (production mode)
```

### Debug Macros Available
```c
// For debug messages with no format arguments
DEBUG_PRINT_STR("Simple debug message\n");

// For debug messages with format arguments  
DEBUG_PRINT("User %s logged in with ID %d\n", username, user_id);

// For error messages with no format arguments
ERROR_PRINT_STR("Critical system error\n");

// For error messages with format arguments
ERROR_PRINT("Failed to connect to %s:%d\n", host, port);
```

### Usage Examples

#### Production Mode (Silent)
```bash
$ ./my-c-express-app
# No framework output - completely silent
# Only your application's chosen output appears
```

#### Debug Mode (Verbose)
```bash
$ C_EXPRESS_DEBUG=1 ./my-c-express-app
[C-Express DEBUG] express_init: initialized request context
[C-Express DEBUG] router_handle: method=GET, path=/api/users
[C-Express DEBUG] route_pattern_match: matching '/api/users' against pattern '/api/users'
[C-Express DEBUG] route_pattern_match: MATCH! captured 0 parameters
# Your application output intermixed with framework debug info
```

#### Error Reporting (Always Enabled)
```bash
$ ./my-c-express-app
[C-Express ERROR] JSON Parse Error: Unexpected token at line 5
[C-Express ERROR] Unhandled error caught: Route handler failed
# Error output appears regardless of debug setting
```

### Remaining Printf Statements (Intentional)
The following printf statements remain and are **intentionally preserved**:

#### User-Callable Convenience Functions (3 statements)
```c
// src/core/app.c:487
void app_print_routes(App *app) {
    char *routes_str = app_get_routes_string(app);
    if (routes_str) {
        printf("%s", routes_str);  // ← User explicitly requested output
        free(routes_str);
    }
}

// src/core/route.c:1296  
void print_route_info(const Route *route) {
    char *info_str = get_route_info_string(route);
    if (info_str) {
        printf("%s", info_str);    // ← User explicitly requested output
        free(info_str);
    }
}

// src/parsers/json.c:978
void json_print_value(JsonValue *value, int indent) {
    char *json_str = json_value_to_string(value, indent);
    if (json_str) {
        printf("%s", json_str);    // ← User explicitly requested output
        free(json_str);
    }
}
```

#### Why These Printf Statements Are Being left
1. **User-controlled**: Only execute when users explicitly call these convenience functions
2. **Not automatic framework output**: Framework never calls these internally
3. **API design pattern**: Provides both string-based (primary) and print-based (convenience) APIs
4. **Zero framework noise**: Framework produces no output unless user code specifically requests it

### Framework Output Philosophy
**The framework provides infrastructure, applications control output.**

- **Framework responsibility**: Provide functionality and optional debug information
- **Application responsibility**: Decide what, when, where, and how to output information
- **User choice**: Use string-based APIs for custom handling or print-based APIs for convenience