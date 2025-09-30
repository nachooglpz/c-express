#!/bin/bash

echo "C-Express Framework - Quick Start"
echo "===================================="
echo

# Check if we're in the right directory
if [ ! -f "Makefile" ] || [ ! -d "src" ]; then
    echo "Error: Run this script from the c-express root directory"
    echo "   Make sure you've cloned the repository correctly."
    exit 1
fi

echo "Repository structure looks good"
echo

# Build the library
echo "Building the C-Express library..."
if make clean > /dev/null 2>&1 && make all > /dev/null 2>&1; then
    echo "Library built successfully"
else
    echo "✗ Build failed. Check your development environment."
    echo "   Requirements: GCC, Make, Linux/Unix"
    exit 1
fi

echo

# Test a simple unit test
echo "Running a quick test..."
if make test-minimal_pattern > /dev/null 2>&1; then
    echo "Tests are working"
else
    echo "Test failed. There might be an issue with the build."
    exit 1
fi

echo
echo "Setup Complete! Here's what you can do:"
echo
echo "Read the documentation:"
echo "cat README.md"
echo "cat tests/README.md"
echo
echo "Run tests:"
echo "./run_test.sh help        # Show all available tests"
echo "./run_test.sh pattern     # Run a simple unit test"
echo "./run_test.sh streaming   # Start a streaming server (port 3000)"
echo
echo "Build commands:"
echo "make test-minimal_pattern # Quick unit test"
echo "make test-streaming       # Full server example"
echo "make help                 # Show all build targets"
echo
echo "Example server tests to try:"
echo "make test-validation_simple   # Route validation (port 3000)"
echo "make test-json_parsing        # JSON parsing (port 3000)"
echo "make test-form_data           # Form handling (port 3000)"
echo
echo "Tip: Server tests start HTTP servers. Use Ctrl+C to stop them."
echo "Try: curl http://localhost:3000/info"
echo
echo "Framework features implemented:"
echo "- Request streaming and large body handling"
echo "- Advanced route pattern matching"
echo "- JSON and form data parsing"
echo "- Content negotiation"
echo "- Middleware system with error handling"
echo "- Memory safety with AddressSanitizer"
echo
echo "Ready to explore the C-Express framework!"
