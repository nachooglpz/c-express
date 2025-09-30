#!/bin/bash

echo "=== C-Express Test Suite Setup ==="
echo "Date: $(date)"
echo "Working Directory: $(pwd)"
echo

echo "1. Checking required files..."
echo "   - Makefile: $(test -f Makefile && echo "[OK]" || echo "[FAIL]")"
echo "   - src/ directory: $(test -d src && echo "[OK]" || echo "[FAIL]")"
echo "   - tests/ directory: $(test -d tests && echo "[OK]" || echo "[FAIL]")"
echo

echo "2. Available test files:"
for test_file in tests/*.c; do
    if [ -f "$test_file" ]; then
        basename "$test_file" .c | sed 's/test_/   - /'
    fi
done
echo

echo "3. Building library..."
if make clean && make all > build.log 2>&1; then
    echo "   Library built successfully"
else
    echo "   Library build failed. Check build.log"
    exit 1
fi
echo

echo "4. Testing available unit tests..."

# Test the pattern test (known to work)
echo "   Testing minimal_pattern..."
if make test-minimal_pattern > test_minimal.log 2>&1; then
    echo "     minimal_pattern test passed"
else
    echo "     minimal_pattern test failed"
fi

# Test memory management
echo "   Testing request memory management..."
if make test-request_memory > test_request_memory.log 2>&1; then
    echo "     request_memory test passed"
else
    echo "     request_memory test failed"
fi

# Test if streaming test compiles (it's a server test, so won't complete)
echo "   Testing streaming (build only)..."
if make build/tests/test_streaming > test_streaming_build.log 2>&1; then
    echo "     streaming test builds successfully"
else
    echo "     streaming test build failed"
fi

echo
echo "5. Example commands to run tests:"
echo "   Unit tests (safe, no servers):"
echo "     make test-minimal_pattern"
echo "     make test-json_simple"
echo "     make test-request_memory"
echo "     make test-modules_memory"
echo "     make test-json_memory"
echo "     make test-response_memory"
echo "     make test-error_memory"
echo
echo "   Test groups:"
echo "     make test-unit          # All unit tests"
echo "     make test-memory        # All memory tests"
echo "     make test-servers       # All server tests (will hang)"
echo
echo "   Memory audit:"
echo "     ./memory_audit.sh       # Comprehensive memory leak audit"
echo
echo "   Server/Integration tests (will start servers):"
echo "     make test-streaming"
echo "     make test-json_parsing"  
echo "     make test-form_data"
echo "     make test-validation_simple"
echo "     make test-patterns"
echo "     make test-content_negotiation"
echo "     make test-route_metadata"
echo "     make test-subrouter"
echo
echo "   All tests:"
echo "     make test"
echo
echo "=== Setup Complete ==="
