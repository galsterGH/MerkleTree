# Merkle Tree Library Documentation {#mainpage}

Welcome to the **Merkle Tree Library** documentation! This library provides a high-performance, thread-safe C implementation of n-ary Merkle trees with SHA-256 hashing.

## 🌟 Key Features

- **High Performance**: Optimized C implementation with minimal memory overhead
- **Thread-Safe**: Full read-write locking support for concurrent operations
- **Flexible**: Configurable branching factors (binary, ternary, or any n-ary tree)
- **Cross-Platform**: Works on Windows, macOS, and Linux
- **Memory Safe**: Comprehensive memory management with leak detection
- **Production Ready**: Extensive test suite with 100+ unit tests
- **Language Bindings**: Python ctypes example included

## 📚 Quick Navigation

| Section | Description |
|---------|-------------|
| @ref getting_started | Installation and basic usage |
| @ref api_overview | Comprehensive API reference |
| @ref examples | Code examples and tutorials |
| @ref threading | Thread safety and locking |

## 🚀 Quick Start

```c
#include "Merkle.h"

// Create sample data
const char *data[] = {"Hello", "World", "Merkle", "Tree"};
size_t sizes[] = {5, 5, 6, 4};

// Build a binary Merkle tree
merkle_tree_t *tree = create_merkle_tree(
    (const void**)data, sizes, 4, 2);

// Get the root hash
unsigned char root_hash[32];
get_tree_hash(tree, root_hash);

// Generate a proof for the first element
merkle_proof_t *proof = NULL;
generate_proof_from_index(tree, 0, &proof);

// Clean up
dealloc_merkle_tree(tree);
```

## 🏗️ Architecture Overview

The library consists of several key components:

### Core Data Structures
- **merkle_tree_t**: The main tree structure with thread-safe operations
- **merkle_node_t**: Individual nodes containing hashes and children
- **merkle_proof_t**: Cryptographic proofs for data integrity verification

### Key Modules
- **Merkle.h**: Main public API for tree operations
- **MerkleQueue.h**: Internal queue for efficient tree construction
- **locking.h**: Cross-platform read-write locking primitives
- **merkle_utils.h**: Memory management and utility functions

### Algorithm Features
- **Queue-based Construction**: Efficient bottom-up tree building
- **SHA-256 Hashing**: Cryptographically secure hash function
- **Proof Generation**: Creates merkle proofs for data verification
- **Memory Safety**: Custom allocation tracking with debug support

## 🔧 Build Requirements

- **C11 Compiler**: GCC 7+ or Clang 6+
- **OpenSSL 3.x**: For SHA-256 cryptographic operations
- **CMake 3.10+**: For building (optional)
- **Valgrind**: For memory leak detection during testing

## 📋 Testing

The library includes comprehensive testing:
- **100+ Unit Tests**: Covering all major functionality
- **Threading Tests**: Concurrent operation validation
- **Memory Leak Tests**: Valgrind integration
- **Performance Tests**: Benchmarking and stress testing

Run tests with:
```bash
cd tests && ./run_tests.sh --memory
```

## 🐍 Language Bindings

Python bindings are available using ctypes:

```python
from python_ctypes_example import build_tree, get_root_hash

# Create tree from Python
tree = build_tree([b'hello', b'world'])
hash_bytes = get_root_hash(tree)
print(f"Root hash: {hash_bytes.hex()}")
```

## 📖 Further Reading

- @ref getting_started "Getting Started Guide"
- @ref api_overview "Complete API Reference"
- @ref examples "Code Examples and Tutorials"
- **GitHub Repository**: [Merkle Tree Implementation](https://github.com/your-repo/merkle-tree)

---

**Author**: Guy Alster  
**License**: MIT License  
**Version**: 1.0.0