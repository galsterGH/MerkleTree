# Getting Started {#getting_started}

This guide will help you get up and running with the Merkle Tree Library quickly.

## 📦 Installation

### Prerequisites

Before building the library, ensure you have the following installed:

- **C11-compatible compiler** (GCC 7+, Clang 6+, or MSVC 2019+)
- **OpenSSL 3.x development libraries**
- **Make** (for building)
- **Valgrind** (optional, for memory testing)

### Platform-Specific Setup

#### macOS
```bash
# Install OpenSSL via Homebrew
brew install openssl@3

# Clone and build
git clone <repository-url>
cd merkle-tree
make all
```

#### Ubuntu/Debian
```bash
# Install dependencies
sudo apt-get install build-essential libssl-dev valgrind

# Clone and build
git clone <repository-url>
cd merkle-tree
make all
```

#### Windows (MSYS2/MinGW)
```bash
# Install dependencies via pacman
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-openssl

# Clone and build
git clone <repository-url>
cd merkle-tree
make all
```

### Building the Library

The library provides several build targets:

```bash
# Build shared library
make lib

# Build and run tests
make test

# Build with memory leak detection
make test-memory

# Build documentation
make docs

# Clean build artifacts
make clean
```

### Verify Installation

Test the installation by running the test suite:

```bash
cd tests
./run_tests.sh

# Expected output:
# Starting Merkle Tree Unit Tests
# ================================
# 
# Running test_create_null_data...
# PASS: test_create_null_data
# ...
# === Test Summary ===
# Total tests: 45
# Passed: 45
# Failed: 0
```

## 🔧 Basic Usage

### 1. Include Headers

```c
#include "Merkle.h"
#include <stdio.h>
#include <string.h>
```

### 2. Create Your First Tree

```c
int main() {
    // Sample data to include in the tree
    const char *data[] = {
        "Transaction 1",
        "Transaction 2", 
        "Transaction 3",
        "Transaction 4"
    };
    
    // Calculate sizes of each data block
    size_t sizes[] = {
        strlen(data[0]),
        strlen(data[1]),
        strlen(data[2]),
        strlen(data[3])
    };
    
    // Create a binary Merkle tree (branching factor = 2)
    merkle_tree_t *tree = create_merkle_tree(
        (const void**)data,  // Data pointers
        sizes,               // Data sizes
        4,                   // Number of elements
        2                    // Branching factor (binary tree)
    );
    
    if (!tree) {
        printf("Failed to create Merkle tree\\n");
        return 1;
    }
    
    printf("✓ Merkle tree created successfully\\n");
    
    // Always clean up when done
    dealloc_merkle_tree(tree);
    return 0;
}
```

### 3. Get the Root Hash

```c
// Get the tree's root hash
unsigned char root_hash[32];  // SHA-256 produces 32-byte hashes
merkle_error_t result = get_tree_hash(tree, root_hash);

if (result == MERKLE_SUCCESS) {
    printf("Root hash: ");
    for (int i = 0; i < 32; i++) {
        printf("%02x", root_hash[i]);
    }
    printf("\\n");
} else {
    printf("Failed to get root hash\\n");
}
```

### 4. Generate and Verify Proofs

```c
// Generate a proof for the first element (index 0)
merkle_proof_t *proof = NULL;
result = generate_proof_from_index(tree, 0, &proof);

if (result == MERKLE_SUCCESS && proof) {
    printf("✓ Proof generated for element 0\\n");
    printf("  Proof path length: %zu\\n", proof->path_length);
    printf("  Leaf index: %zu\\n", proof->leaf_index);
    printf("  Branching factor: %zu\\n", proof->branching_factor);
    
    // Verify the proof
    result = verify_proof(proof, root_hash, data[0], sizes[0]);
    if (result == MERKLE_SUCCESS) {
        printf("✓ Proof verification successful\\n");
    } else {
        printf("✗ Proof verification failed\\n");
    }
    
    // Clean up proof (implementation detail - see examples)
    // ... proof cleanup code ...
} else {
    printf("Failed to generate proof\\n");
}
```

## 🔄 Common Patterns

### Error Handling

Always check return values for proper error handling:

```c
merkle_error_t result = get_tree_hash(tree, hash);
switch (result) {
    case MERKLE_SUCCESS:
        // Success - proceed with hash
        break;
    case MERKLE_NULL_ARG:
        printf("Error: NULL argument passed\\n");
        break;
    case MERKLE_BAD_ARG:
        printf("Error: Invalid tree structure\\n");
        break;
    default:
        printf("Error: Unknown error code %d\\n", result);
        break;
}
```

### Memory Management

The library follows RAII principles - always pair creation with cleanup:

```c
// Create
merkle_tree_t *tree = create_merkle_tree(data, sizes, count, bf);
if (!tree) {
    // Handle error
    return;
}

// Use tree...

// Always clean up
dealloc_merkle_tree(tree);
```

### Thread Safety

The library is thread-safe for concurrent reads:

```c
// Multiple threads can safely call these simultaneously:
get_tree_hash(tree, hash);
generate_proof_from_index(tree, index, &proof);

// Tree creation/destruction should be single-threaded
```

## 🎯 Next Steps

Now that you have the basics working, explore these advanced topics:

1. **@ref api_overview** - Complete API reference
2. **@ref examples** - More complex examples and use cases
3. **Threading Guide** - Concurrent operation patterns
4. **Performance Tuning** - Optimization techniques
5. **Python Bindings** - Using the library from Python

## 🐛 Troubleshooting

### Common Issues

**"Library not found" errors:**
- Ensure OpenSSL is installed and in your library path
- Check that `libmerkle.so` was built successfully

**Segmentation faults:**
- Always check return values from `create_merkle_tree()`
- Ensure data pointers are valid before calling tree functions
- Run with valgrind for detailed memory error detection

**Test failures:**
- Verify OpenSSL version is 3.x
- Check that all dependencies are installed
- Run `make clean && make test` to rebuild

### Getting Help

- Check the **@ref examples** page for working code samples
- Review the **API Reference** for detailed function documentation
- Run tests with `./run_tests.sh --verbose` for detailed output
- Enable debug mode by defining `MERKLE_DEBUG` during compilation