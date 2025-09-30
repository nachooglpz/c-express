# C-Express Test Suite

This directory contains comprehensive tests for the C-Express web framework. The tests serve as both validation and examples of how to use the framework.

## Quick Start

After cloning the repository, run:

```bash
./test_setup.sh
```

This will build the library and show you available tests.

## Test Categories

### Unit Tests (Standalone)
These tests can be run independently and don't start servers:

- `make test-minimal_pattern` - Route pattern compilation and matching
- `make test-json_simple` - JSON parser functionality 
- `make test-json_memory` - Complex JSON parsing with memory management
- `make test-request_memory` - Request memory management and cleanup
- `make test-response_memory` - Response module memory management  
- `make test-modules_memory` - Streaming, Router, and Error module memory management
- `make test-error_memory` - Error handling memory management
- `make test-response_api` - Response API functionality

### Memory Safety Tests
These tests validate proper memory management across the framework:

- `make test-request_memory` - Validates request JSON/form error handling cleanup
- `make test-modules_memory` - Tests streaming, router mounting, and error context memory
- `make test-json_memory` - Complex nested JSON parsing without leaks
- `make test-response_memory` - Response create/destroy cycle validation
- `make test-error_memory` - Error and ErrorContext memory management

All memory tests use AddressSanitizer to detect leaks and run automatically in CI.

### Integration Tests (Server Examples)
These tests start HTTP servers and serve as practical examples:

- `make test-streaming` - HTTP request streaming and large body handling
- `make test-validation_simple` - Route validation and constraints
- `make test-json_parsing` - JSON request parsing and validation
- `make test-form_data` - Form data parsing and handling
- `make test-patterns` - Advanced route patterns
- `make test-content_negotiation` - Content negotiation
- `make test-route_metadata` - Route metadata and OpenAPI
- `make test-subrouter` - Subrouter mounting

### Development Tests
- `make test-negotiation_minimal` - Standalone content negotiation

## Running Tests

### Individual Tests
```bash
# Run a specific test
make test-minimal_pattern

# Run a server test (use Ctrl+C to stop)
make test-streaming
```

### All Tests
```bash
# Run all unit tests (safe, no servers)
make test-unit

# Run all memory tests 
make test-memory

# Run all tests (note: server tests will hang until interrupted)
make test
```

### Memory Audit
```bash
# Run comprehensive memory leak audit
./memory_audit.sh
```

### Building Tests Only
```bash
# Build a test without running it
make build/tests/test_streaming
```

## Server Test Usage

For server tests, you'll see startup information and suggested curl commands. For example:

```bash
make test-streaming
# Server starts on port 3000 with test endpoints
# Use: curl http://localhost:3000/info
```

Common test endpoints:
- `/info` - Server information and available endpoints
- `/test/*` - Various test endpoints depending on the test

## Test Structure

Each test demonstrates specific framework features:

1. **Streaming** (`test_streaming.c`) - Shows the complete streaming implementation for large uploads
2. **Validation** (`test_validation_simple.c`) - Demonstrates route parameter validation
3. **JSON** (`test_json_parsing.c`) - Shows JSON request parsing and schema validation
4. **Forms** (`test_form_data.c`) - Demonstrates form data handling
5. **Patterns** (`test_patterns.c`) - Advanced route pattern matching
6. **Content Negotiation** (`test_content_negotiation.c`) - HTTP content negotiation

## Building From Scratch

If you encounter build issues:

```bash
make clean
make all
make test-minimal_pattern  # Test a simple unit test
```

## Framework Features Demonstrated

- ✓ Request streaming and large body handling
- ✓ Route pattern matching with parameters
- ✓ JSON parsing and validation
- ✓ Form data handling
- ✓ Content negotiation
- ✓ Error handling and middleware
- ✓ Response API with headers and status codes
- ✓ Route validation and constraints

## Known Issues

- Some Makefile warnings about duplicate targets (cosmetic issue)
- All major memory leaks have been fixed
- Memory audit confirms zero leaks in critical request/response cycles

## Memory Safety

The framework has comprehensive memory leak detection:
- AddressSanitizer integration for all tests
- Automated memory audit script (`./memory_audit.sh`)
- Comprehensive request cleanup via `request_destroy()`
- Validated streaming, routing, and error handling memory management

## Contributing

When adding new tests:
1. Create `test_[feature].c` in the `tests/` directory
2. Follow existing patterns for handlers and main functions
3. Add documentation for new endpoints or features
4. Tests should compile and run after `make clean && make all`
