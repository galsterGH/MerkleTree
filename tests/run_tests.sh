#!/bin/bash

# Test runner script for Merkle Tree tests
# Author: Guy Alster
# Date: 2025-06-01

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
}

print_error() {
    echo -e "${RED}[FAIL]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

# Check if we're in the tests directory
if [ ! -f "test_merkle_tree.c" ]; then
    print_error "Please run this script from the tests directory"
    exit 1
fi

# Check if OpenSSL is available
if ! pkg-config --exists openssl 2>/dev/null; then
    print_warning "pkg-config for OpenSSL not found, using manual paths"
fi

# Parse command line arguments
MEMORY_TEST=false
DEBUG_TEST=false
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -m|--memory)
            MEMORY_TEST=true
            shift
            ;;
        -d|--debug)
            DEBUG_TEST=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -m, --memory    Run memory leak tests (requires valgrind)"
            echo "  -d, --debug     Run with debug output enabled"
            echo "  -v, --verbose   Verbose output"
            echo "  -h, --help      Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

print_status "Starting Merkle Tree Test Suite"
echo "=================================="

# Clean previous builds
print_status "Cleaning previous builds..."
make clean > /dev/null 2>&1

# Build tests
print_status "Building tests..."
if [ "$VERBOSE" = true ]; then
    make all
    BUILD_RESULT=$?
else
    BUILD_OUTPUT=$(make all 2>&1)
    BUILD_RESULT=$?
    
    if [ $BUILD_RESULT -ne 0 ]; then
        echo "$BUILD_OUTPUT"
    fi
fi

if [ $BUILD_RESULT -ne 0 ]; then
    print_error "Build failed!"
    print_status "Build errors shown above"
    exit 1
fi

print_success "Build completed successfully"

# Run appropriate test suite
if [ "$MEMORY_TEST" = true ]; then
    print_status "Running memory leak tests..."
    
    # Check if valgrind is available
    if ! command -v valgrind &> /dev/null; then
        print_error "Valgrind not found! Please install valgrind to run memory tests"
        print_status "On macOS: brew install valgrind"
        print_status "On Ubuntu: sudo apt-get install valgrind"
        exit 1
    fi
    
    make test-memory
    
elif [ "$DEBUG_TEST" = true ]; then
    print_status "Running debug tests..."
    make test-debug
    
else
    print_status "Running standard tests..."
    if [ "$VERBOSE" = true ]; then
        make test
    else
        # Capture output and show summary
        TEST_OUTPUT=$(make test 2>&1)
        echo "$TEST_OUTPUT"
        
        # Extract test results
        if echo "$TEST_OUTPUT" | grep -q "Failed: 0"; then
            print_success "All tests passed!"
        else
            FAILED_COUNT=$(echo "$TEST_OUTPUT" | grep "Failed:" | sed 's/Failed: //')
            print_error "$FAILED_COUNT test(s) failed"
        fi
    fi
fi

TEST_EXIT_CODE=$?

echo ""
print_status "Test execution completed"

# Clean up build artifacts if tests passed
if [ $TEST_EXIT_CODE -eq 0 ]; then
    print_status "Cleaning up build artifacts..."
    make clean > /dev/null 2>&1
    print_success "Test suite completed successfully"
else
    print_error "Some tests failed (exit code: $TEST_EXIT_CODE)"
    print_status "Build artifacts preserved for debugging"
fi

exit $TEST_EXIT_CODE