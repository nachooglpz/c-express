#!/bin/bash

# C-Express Test Runner
# Quick access to all tests with descriptions

show_help() {
    echo "C-Express Test Runner"
    echo "===================="
    echo
    echo "Usage: $0 [test-name]"
    echo
    echo "Available tests:"
    echo
    echo "Unit Tests (standalone):"
    echo "  pattern       - Route pattern compilation and matching"
    echo "  response      - Response API functionality"
    echo "  json          - JSON parser"
    echo "  request-mem   - Request memory management"
    echo "  modules-mem   - Streaming/Router/Error memory management"
    echo "  json-mem      - Complex JSON memory management"
    echo "  response-mem  - Response memory management"
    echo "  error-mem     - Error handling memory management"
    echo
    echo "Test Groups:"
    echo "  unit          - Run all unit tests"
    echo "  memory        - Run all memory tests"
    echo "  audit         - Run comprehensive memory audit"
    echo
    echo "Server Tests (start HTTP servers):"
    echo "  streaming     - Request streaming and large body handling (port 8080)"
    echo "  validation    - Route validation and constraints (port 3000)"
    echo "  json-parsing  - JSON request parsing and validation (port 3000)"
    echo "  form-data     - Form data parsing and handling (port 3000)"
    echo "  patterns      - Advanced route patterns (port 3000)"
    echo "  negotiation   - Content negotiation (port 3000)"
    echo "  metadata      - Route metadata and OpenAPI (port 3000)"
    echo "  subrouter     - Subrouter mounting (port 3000)"
    echo
    echo "Special commands:"
    echo "  build         - Build library only"
    echo "  clean         - Clean and rebuild everything"
    echo "  all           - Run all tests (server tests will hang)"
    echo "  help          - Show this help"
    echo
    echo "Examples:"
    echo "  $0 pattern"
    echo "  $0 streaming"
    echo "  $0 clean"
}

run_test() {
    local test_name="$1"
    
    case "$test_name" in
        pattern)
            echo "Running route pattern test..."
            make test-minimal_pattern
            ;;
        response)
            echo "Running response API test..."
            make test-response_api
            ;;
        json)
            echo "Running JSON parser test..."
            make test-json_simple
            ;;
        request-mem)
            echo "Running request memory management test..."
            make test-request_memory
            ;;
        modules-mem)
            echo "Running modules memory management test..."
            make test-modules_memory
            ;;
        json-mem)
            echo "Running JSON memory management test..."
            make test-json_memory
            ;;
        response-mem)
            echo "Running response memory management test..."
            make test-response_memory
            ;;
        error-mem)
            echo "Running error memory management test..."
            make test-error_memory
            ;;
        unit)
            echo "Running all unit tests..."
            make test-unit
            ;;
        memory)
            echo "Running all memory tests..."
            make test-memory
            ;;
        audit)
            echo "Running comprehensive memory audit..."
            ./memory_audit.sh
            ;;
        streaming)
            echo "Starting streaming test server on port 3000..."
            echo "Use Ctrl+C to stop the server."
            make test-streaming
            ;;
        validation)
            echo "Starting validation test server on port 3000..."
            echo "Use Ctrl+C to stop the server."
            make test-validation_simple
            ;;
        json-parsing)
            echo "Starting JSON parsing test server on port 3000..."
            echo "Use Ctrl+C to stop the server."
            make test-json_parsing
            ;;
        form-data)
            echo "Starting form data test server on port 3000..."
            echo "Use Ctrl+C to stop the server."
            make test-form_data
            ;;
        patterns)
            echo "Starting route patterns test server on port 3000..."
            echo "Use Ctrl+C to stop the server."
            make test-patterns
            ;;
        negotiation)
            echo "Starting content negotiation test server on port 3000..."
            echo "Use Ctrl+C to stop the server."
            make test-content_negotiation
            ;;
        metadata)
            echo "Starting route metadata test server on port 3000..."
            echo "Use Ctrl+C to stop the server."
            make test-route_metadata
            ;;
        subrouter)
            echo "Starting subrouter test server on port 3000..."
            echo "Use Ctrl+C to stop the server."
            make test-subrouter
            ;;
        build)
            echo "Building library..."
            make clean && make all
            ;;
        clean)
            echo "Cleaning and rebuilding everything..."
            make clean && make all && echo "Build complete. Try: $0 pattern"
            ;;
        all)
            echo "Running all tests..."
            echo "WARNING: Server tests will start servers that need to be stopped manually."
            make test
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            echo "Unknown test: $test_name"
            echo "Use '$0 help' to see available tests."
            exit 1
            ;;
    esac
}

# Main script
if [ $# -eq 0 ]; then
    show_help
    exit 0
fi

# Ensure we're in the right directory
if [ ! -f "Makefile" ] || [ ! -d "src" ]; then
    echo "Error: This script must be run from the c-express root directory."
    exit 1
fi

run_test "$1"
