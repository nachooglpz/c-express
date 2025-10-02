#!/bin/bash

echo "Setting up C-Express development environment..."

# Check if Node.js is installed
if ! command -v node &> /dev/null; then
    echo "Error: Node.js is not installed. Please install Node.js 16 or later."
    exit 1
fi

# Check Node.js version
NODE_VERSION=$(node --version | cut -d'v' -f2 | cut -d'.' -f1)
if [ "$NODE_VERSION" -lt 16 ]; then
    echo "Error: Node.js version 16 or later is required. Current version: $(node --version)"
    exit 1
fi

# Install npm dependencies
echo "Installing npm dependencies..."
npm install

# Build the native addon
echo "Building native addon..."
npm run build

if [ $? -eq 0 ]; then
    echo "✓ Build successful!"
    
    # Run basic tests
    echo "Running tests..."
    npm test
    
    if [ $? -eq 0 ]; then
        echo "✓ All tests passed!"
        echo ""
        echo "C-Express is ready to use!"
        echo ""
        echo "Try running a simple example:"
        echo "  node examples/hello-world.js"
    else
        echo "⚠ Tests failed, but build was successful"
    fi
else
    echo "✗ Build failed!"
    echo ""
    echo "Common issues:"
    echo "1. Make sure you have build tools installed:"
    echo "   - Linux: sudo apt-get install build-essential"
    echo "   - macOS: xcode-select --install"
    echo "   - Windows: npm install --global windows-build-tools"
    echo ""
    echo "2. Make sure Python is available (required by node-gyp)"
    echo ""
    echo "3. Try cleaning and rebuilding:"
    echo "   npm run clean && npm run build"
    
    exit 1
fi
