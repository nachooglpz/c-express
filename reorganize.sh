#!/bin/bash

# Script to reorganize c-express project structure
# Creates new directory structure and moves existing files

echo "Creating directory structure..."

# Create main directories
mkdir -p src/core
mkdir -p src/http
mkdir -p src/parsers
mkdir -p tests

echo "Moving files to their new locations..."

# Move core framework files to src/core/
echo "Moving core framework files..."
mv app.c src/core/
mv app.h src/core/
mv router.c src/core/
mv router.h src/core/
mv route.c src/core/
mv route.h src/core/
mv layer.c src/core/
mv layer.h src/core/

# Move HTTP handling files to src/http/
echo "Moving HTTP handling files..."
mv request.c src/http/
mv request.h src/http/
mv response.c src/http/
mv response.h src/http/
mv error.c src/http/
mv error.h src/http/

# Move parser files to src/parsers/
echo "Moving parser files..."
mv json.c src/parsers/
mv json.h src/parsers/
mv form.c src/parsers/
mv form.h src/parsers/

# Move test files to tests/
echo "Moving test files..."
mv test_*.c tests/

# Move compiled test executables to tests/
echo "Moving compiled test executables..."
mv test_form_data tests/ 2>/dev/null || true
mv test_json_parsing tests/ 2>/dev/null || true
mv test_json_simple tests/ 2>/dev/null || true
mv test_minimal_pattern tests/ 2>/dev/null || true
mv test_patterns tests/ 2>/dev/null || true
mv test_route_metadata tests/ 2>/dev/null || true
mv test_subrouter tests/ 2>/dev/null || true
mv test_validation_simple tests/ 2>/dev/null || true

echo "Reorganization complete!"
echo ""
echo "New structure:"
echo "src/"
echo "├── core/     (app, router, route, layer)"
echo "├── http/     (request, response, error)"
echo "└── parsers/  (json, form)"
echo "tests/        (all test files)"
echo ""
echo "Files remaining in root:"
ls -la | grep -E '\.(c|h|o)$' || echo "No source files remaining in root"
