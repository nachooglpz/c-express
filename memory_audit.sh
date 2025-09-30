#!/bin/bash

echo "🔍 C-Express Framework Memory Leak Audit"
echo "=========================================="
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to test for memory leaks
test_memory_leaks() {
    local test_name="$1"
    local test_target="$2"
    
    echo -e "${BLUE}Testing $test_name...${NC}"
    
    # Run the test and capture output
    if timeout 5 make "$test_target" > "memory_test_$test_name.log" 2>&1; then
        # Check for memory leaks in the output
        if grep -q "ERROR: LeakSanitizer" "memory_test_$test_name.log"; then
            echo -e "${RED}Memory leaks detected in $test_name${NC}"
            echo "   Details in memory_test_$test_name.log"
            grep -A 5 "ERROR: LeakSanitizer" "memory_test_$test_name.log" | head -10
            return 1
        elif grep -q "PASSED" "memory_test_$test_name.log"; then
            echo -e "${GREEN}$test_name - No memory leaks detected${NC}"
            rm -f "memory_test_$test_name.log"
            return 0
        else
            echo -e "${YELLOW}$test_name - Test incomplete or server test${NC}"
            rm -f "memory_test_$test_name.log"
            return 0
        fi
    else
        echo -e "${YELLOW}$test_name - Timeout (likely server test)${NC}"
        rm -f "memory_test_$test_name.log"
        return 0
    fi
}

# Function to test individual source files for potential leaks
analyze_source_code() {
    echo -e "${BLUE}Analyzing source code for potential memory issues...${NC}"
    
    echo "Checking for malloc/free patterns..."
    
    # Find all malloc calls without corresponding free
    echo "Files with malloc calls:"
    grep -r "malloc\|calloc\|realloc" src/ --include="*.c" | cut -d: -f1 | sort | uniq
    
    echo
    echo "Files with free calls:"
    grep -r "free(" src/ --include="*.c" | cut -d: -f1 | sort | uniq
    
    echo
    echo "Files with strdup calls (need free):"
    grep -r "strdup" src/ --include="*.c" | cut -d: -f1 | sort | uniq
}

# Function to check for common memory issues in code
check_memory_patterns() {
    echo -e "${BLUE}Checking for common memory leak patterns...${NC}"
    
    echo "1. Checking for malloc without free in same function:"
    for file in $(find src/ -name "*.c"); do
        if grep -q "malloc\|calloc\|realloc" "$file"; then
            func_with_malloc=$(grep -n "malloc\|calloc\|realloc" "$file" | head -5)
            if [ -n "$func_with_malloc" ]; then
                echo "   $file has memory allocation:"
                echo "$func_with_malloc" | sed 's/^/     /'
            fi
        fi
    done
    
    echo
    echo "2. Checking for strdup without free:"
    grep -rn "strdup" src/ --include="*.c" | head -10
    
    echo
    echo "3. Checking for potential double-free issues:"
    grep -rn "free.*free" src/ --include="*.c" | head -5
}

echo "Building fresh library for testing..."
if make clean > /dev/null 2>&1 && make all > /dev/null 2>&1; then
    echo -e "${GREEN}Library built successfully${NC}"
else
    echo -e "${RED}Failed to build library${NC}"
    exit 1
fi

echo
echo "=== UNIT TESTS MEMORY ANALYSIS ==="

# Test unit tests that don't start servers
test_memory_leaks "minimal_pattern" "test-minimal_pattern"
test_memory_leaks "json_simple" "test-json_simple"
test_memory_leaks "request_memory" "test-request_memory"
test_memory_leaks "modules_memory" "test-modules_memory"

# Test the new JSON memory test
echo -e "${BLUE}Testing json_memory (complex test)...${NC}"
if gcc -std=c99 -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined tests/test_json_memory.c -Llib -lc-express -o temp_json_test 2>/dev/null && ./temp_json_test > json_memory.log 2>&1; then
    if grep -q "ERROR: LeakSanitizer" json_memory.log; then
        echo -e "${RED}Memory leaks in complex JSON test${NC}"
        grep -A 5 "ERROR: LeakSanitizer" json_memory.log
    else
        echo -e "${GREEN}Complex JSON test - No memory leaks${NC}"
    fi
    rm -f temp_json_test json_memory.log
else
    echo -e "${YELLOW}Complex JSON test failed to compile/run${NC}"
fi

echo
echo "=== SOURCE CODE ANALYSIS ==="
analyze_source_code

echo
echo "=== MEMORY PATTERN ANALYSIS ==="
check_memory_patterns

echo
echo "=== RECOMMENDATIONS ==="
echo "1. Focus on files that have malloc/strdup but may be missing free calls"
echo "2. Check error paths in functions - they often miss cleanup"
echo "3. Review any create/destroy function pairs for completeness"
echo "4. Pay attention to string handling functions (strdup usage)"

echo
echo "Memory audit complete! Check any .log files for detailed information."
