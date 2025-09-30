# C-Express Web Framework
# =======================================================

# Compiler Configuration
CC := gcc
AR := ar
STRIP := strip
# Version Information
VERSION_MAJOR := 1
VERSION_MINOR := 0
VERSION_PATCH := 0
VERSION := $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)
# Directory Structure
SRCDIR := src
BUILDDIR := build
LIBDIR := lib
TESTDIR := tests
EXAMPLEDIR := examples
DISTDIR := dist
# Source Organization
CORE_SRCDIR := $(SRCDIR)/core
HTTP_SRCDIR := $(SRCDIR)/http
PARSERS_SRCDIR := $(SRCDIR)/parsers
# Compiler Flags
CFLAGS_BASE := -std=c99 -pedantic -Wall -Wextra -Werror
CFLAGS_DEBUG := $(CFLAGS_BASE) -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined
CFLAGS_RELEASE := $(CFLAGS_BASE) -O3 -DNDEBUG -flto
CFLAGS_TEST := $(CFLAGS_DEBUG) -coverage
# Default to debug build
BUILD_TYPE ?= debug
ifeq ($(BUILD_TYPE),release)
    CFLAGS := $(CFLAGS_RELEASE)
else ifeq ($(BUILD_TYPE),test)
    CFLAGS := $(CFLAGS_TEST)
    LDFLAGS := -lgcov --coverage
else
    CFLAGS := $(CFLAGS_DEBUG)
    LDFLAGS := -fsanitize=address -fsanitize=undefined
endif
# Source Files
CORE_SRC := $(wildcard $(CORE_SRCDIR)/*.c)
HTTP_SRC := $(wildcard $(HTTP_SRCDIR)/*.c)
PARSERS_SRC := $(wildcard $(PARSERS_SRCDIR)/*.c)
LIB_SRC := $(CORE_SRC) $(HTTP_SRC) $(PARSERS_SRC)
# Object Files
CORE_OBJ := $(CORE_SRC:$(CORE_SRCDIR)/%.c=$(BUILDDIR)/core/%.o)
HTTP_OBJ := $(HTTP_SRC:$(HTTP_SRCDIR)/%.c=$(BUILDDIR)/http/%.o)
PARSERS_OBJ := $(PARSERS_SRC:$(PARSERS_SRCDIR)/%.c=$(BUILDDIR)/parsers/%.o)
LIB_OBJ := $(CORE_OBJ) $(HTTP_OBJ) $(PARSERS_OBJ)
# Example Files
EXAMPLE_SRC := $(wildcard $(EXAMPLEDIR)/*/main.c)
EXAMPLE_BIN := $(EXAMPLE_SRC:$(EXAMPLEDIR)/%/main.c=$(BUILDDIR)/examples/%)
# Test Files
TEST_SRC := $(wildcard $(TESTDIR)/*.c)
TEST_BIN := $(TEST_SRC:$(TESTDIR)/%.c=$(BUILDDIR)/tests/%)
# Targets
STATIC_LIB := $(LIBDIR)/libc-express.a
SHARED_LIB := $(LIBDIR)/libc-express.so.$(VERSION)
DIST_ARCHIVE := $(DISTDIR)/c-express-$(VERSION).tar.gz
# Default Target
.DEFAULT_GOAL := all
# Main Targets
all: $(STATIC_LIB)
release: clean
	@$(MAKE) BUILD_TYPE=release all
debug: clean  
	@$(MAKE) BUILD_TYPE=debug all
# Examples
examples: $(EXAMPLE_BIN)
$(BUILDDIR)/examples/%: $(EXAMPLEDIR)/%/main.c $(STATIC_LIB) | $(BUILDDIR)/examples
	@echo "Building example $*..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I. -o $@ $< -L$(LIBDIR) -lc-express $(LDFLAGS)
# Static Library
$(STATIC_LIB): $(LIB_OBJ) | $(LIBDIR)
	@echo "Creating static library..."
	$(AR) rcs $@ $^
# Shared Library  
$(SHARED_LIB): $(LIB_OBJ) | $(LIBDIR)
	@echo "Creating shared library..."
	$(CC) -shared -fPIC -Wl,-soname,libc-express.so.$(VERSION_MAJOR) -o $@ $^ $(LDFLAGS)
	cd $(LIBDIR) && ln -sf libc-express.so.$(VERSION) libc-express.so.$(VERSION_MAJOR)
	cd $(LIBDIR) && ln -sf libc-express.so.$(VERSION_MAJOR) libc-express.so
# Object Files - Core
$(BUILDDIR)/core/%.o: $(CORE_SRCDIR)/%.c | $(BUILDDIR)/core
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -fPIC -c $< -o $@
# Object Files - HTTP
$(BUILDDIR)/http/%.o: $(HTTP_SRCDIR)/%.c | $(BUILDDIR)/http
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -fPIC -c $< -o $@
# Object Files - Parsers
$(BUILDDIR)/parsers/%.o: $(PARSERS_SRCDIR)/%.c | $(BUILDDIR)/parsers
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -fPIC -c $< -o $@
# Test Targets
test: $(TEST_BIN)
	@echo "Running test suite..."
	@passed=0; total=0; \
	for test in $(TEST_BIN); do \
		echo "Running $$test..."; \
		total=$$((total + 1)); \
		if ./$$test; then \
			echo "✓ $$test PASSED"; \
			passed=$$((passed + 1)); \
		else \
			echo "✗ $$test FAILED"; \
		fi; \
		echo ""; \
	done; \
	echo "Test Results: $$passed/$$total tests passed"; \
	if [ $$passed -eq $$total ]; then \
		echo "✓ All tests passed!"; \
		exit 0; \
	else \
		echo "✗ Some tests failed!"; \
		exit 1; \
	fi
# Individual Test Compilation
$(BUILDDIR)/tests/%: $(TESTDIR)/%.c $(STATIC_LIB) | $(BUILDDIR)/tests
	@echo "Compiling test $@..."
	$(CC) $(CFLAGS) -o $@ $< -L$(LIBDIR) -lc-express $(LDFLAGS)

# Run individual test by name (e.g., make run-test-streaming)
run-test-%: $(BUILDDIR)/tests/test_%
	@echo "Running test $*..."
	@./$(BUILDDIR)/tests/test_$*

# Quick test runner (e.g., make test-streaming)  
test-%: $(BUILDDIR)/tests/test_%
	@echo "Running test $*..."
	@if ./$(BUILDDIR)/tests/test_$*; then \
		echo "✓ test_$* PASSED"; \
	else \
		echo "✗ test_$* FAILED"; \
		exit 1; \
	fi
# Coverage Report (requires gcov)
coverage: clean
	@echo "Building with coverage instrumentation..."
	@$(MAKE) BUILD_TYPE=test test
	@echo "Generating coverage report..."
	@mkdir -p coverage
	@gcov -r $(LIB_SRC) -o $(BUILDDIR)
	@lcov --capture --directory . --output-file coverage/coverage.info 2>/dev/null || true
	@genhtml coverage/coverage.info --output-directory coverage/html 2>/dev/null || true
	@echo "Coverage report generated in coverage/html/index.html"
# Benchmarks
benchmark: release
	@echo "Running performance benchmarks..."
	@# Add benchmark targets here when implemented
# Static Analysis
analyze:
	@echo "Running static analysis..."
	@command -v cppcheck >/dev/null 2>&1 && cppcheck --enable=all --std=c99 $(SRCDIR)/ || echo "cppcheck not found"
	@command -v scan-build >/dev/null 2>&1 && scan-build make || echo "clang static analyzer not found"
# Memory Leak Detection
memcheck: debug $(MAIN_TARGET)
	@echo "Running memory leak detection..."
	@command -v valgrind >/dev/null 2>&1 && valgrind --leak-check=full --show-leak-kinds=all ./$(MAIN_TARGET) || echo "valgrind not found"
# Installation
install: release $(SHARED_LIB)
	@echo "Installing C-Express..."
	@mkdir -p /usr/local/lib /usr/local/include/c-express
	@cp $(STATIC_LIB) $(SHARED_LIB) /usr/local/lib/
	@cp $(SRCDIR)/core/*.h $(SRCDIR)/http/*.h $(SRCDIR)/parsers/*.h /usr/local/include/c-express/
	@ldconfig 2>/dev/null || true
	@echo "Installation complete!"
uninstall:
	@echo "Uninstalling C-Express..."
	@rm -f /usr/local/lib/libc-express.*
	@rm -rf /usr/local/include/c-express
	@echo "Uninstallation complete!"
# Distribution Package
dist: clean release
	@echo "Creating distribution package..."
	@mkdir -p $(DISTDIR)/c-express-$(VERSION)
	@cp -r $(SRCDIR) $(TESTDIR) examples Makefile README.* LICENSE $(DISTDIR)/c-express-$(VERSION)/ 2>/dev/null || true
	@cp $(STATIC_LIB) $(DISTDIR)/c-express-$(VERSION)/
	@cd $(DISTDIR) && tar -czf c-express-$(VERSION).tar.gz c-express-$(VERSION)
	@echo "Distribution package created: $(DIST_ARCHIVE)"
# Documentation
docs:
	@echo "Generating documentation..."
	@command -v doxygen >/dev/null 2>&1 && doxygen Doxyfile || echo "doxygen not found"
# Development Helpers
format:
	@echo "Formatting code..."
	@command -v clang-format >/dev/null 2>&1 && find $(SRCDIR) -name "*.c" -o -name "*.h" | xargs clang-format -i || echo "clang-format not found"
lint:
	@echo "Running linter..."
	@command -v cpplint >/dev/null 2>&1 && cpplint --recursive $(SRCDIR) || echo "cpplint not found"
# Cleanup
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILDDIR) $(LIBDIR) $(DISTDIR) coverage/
	@rm -f *.o *.gcov *.gcda *.gcno
distclean: clean
	@echo "Deep cleaning..."
	@rm -f *~ .*~ core vgcore.* *.tmp
# Directory Creation
$(BUILDDIR) $(BUILDDIR)/core $(BUILDDIR)/http $(BUILDDIR)/parsers $(BUILDDIR)/tests $(BUILDDIR)/examples:
	@mkdir -p $@
$(LIBDIR) $(DISTDIR):
	@mkdir -p $@

# Ensure directories exist before building
$(LIB_OBJ): | $(BUILDDIR) $(BUILDDIR)/core $(BUILDDIR)/http $(BUILDDIR)/parsers
$(STATIC_LIB): | $(LIBDIR)
$(TEST_BIN): | $(BUILDDIR)/tests
# Help
help:
	@echo "C-Express Build System"
	@echo "======================"
	@echo ""
	@echo "Main Targets:"
	@echo "  all          Build static library (debug)"
	@echo "  release      Build optimized release version"  
	@echo "  debug        Build debug version with sanitizers"
	@echo "  examples     Build example applications"
	@echo "  test         Build and run all tests"
	@echo "  coverage     Generate test coverage report"
	@echo "Libraries:"
	@echo "  $(STATIC_LIB)    Build static library"
	@echo "  $(SHARED_LIB)     Build shared library"
	@echo "Quality Assurance:"
	@echo "  analyze      Run static code analysis"
	@echo "  memcheck     Run memory leak detection"
	@echo "  benchmark    Run performance benchmarks"
	@echo "  format       Format code with clang-format"
	@echo "  lint         Run code linter"
	@echo "Distribution:"
	@echo "  install      Install system-wide"
	@echo "  uninstall    Remove system installation"
	@echo "  dist         Create distribution package"
	@echo "  docs         Generate documentation"
	@echo "Maintenance:"
	@echo "  clean        Remove build artifacts"
	@echo "  distclean    Deep clean including temp files"
	@echo "  help         Show this help message"
	@echo "Build Types:"
	@echo "  make BUILD_TYPE=debug    (default, with sanitizers)"
	@echo "  make BUILD_TYPE=release  (optimized, no debug info)"
	@echo "  make BUILD_TYPE=test     (with coverage instrumentation)"
# Phony Targets
.PHONY: all release debug test coverage benchmark analyze memcheck install uninstall dist docs format lint clean distclean help