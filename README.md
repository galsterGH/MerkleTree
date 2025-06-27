# Merkle Tree Implementation

A robust, n-ary Merkle Tree implementation in C with comprehensive testing and documentation. This cryptographic data structure efficiently verifies data integrity and consistency, making it ideal for blockchain technologies, distributed systems, and cryptographic applications.

## ✨ Features

- **N-ary Merkle Trees**: Configurable branching factor (binary, ternary, or any n-ary tree)
- **SHA-256 Hashing**: Cryptographically secure hashing using OpenSSL
- **Queue-based Construction**: Efficient bottom-up tree building algorithm
- **Memory Safe**: Comprehensive error handling and memory management
- **Opaque API**: Clean public interface with implementation details hidden
- **Proof Verification**: Validate proofs against a known root hash
- **Comprehensive Tests**: Full unit test suite with memory leak detection
- **Documentation**: Complete Doxygen-generated API documentation

## 📁 Project Structure

```
merkle-tree/
├── src/                          # Source files
│   ├── merkle_tree.c            # Core Merkle tree implementation
│   ├── merkle_queue.c           # Queue data structure for tree construction
│   └── merkle_utils.c           # Memory management utilities
├── include/                      # Header files
│   ├── Merkle.h                 # Public Merkle tree API
│   ├── MerkleQueue.h            # Queue interface
│   └── Utils.h                  # Utility functions and macros
├── tests/                       # Test suite
│   ├── test_merkle_tree.c       # Comprehensive unit tests
│   ├── Makefile                 # Test build system
│   └── run_tests.sh             # Test runner script
├── docs/                        # Generated documentation
└── .vscode/                     # VS Code configuration
```

## 🚀 Quick Start

### Prerequisites

- **C Compiler**: clang or gcc with C11 support
- **OpenSSL 3.x**: For SHA-256 hashing
- **Make**: For building tests
- **Valgrind** (optional): For memory leak detection

#### macOS Installation:

```bash
# Install dependencies
brew install openssl@3 make

# Install Valgrind (optional)
brew install valgrind
```

#### Ubuntu Installation:

```bash
sudo apt-get update
sudo apt-get install build-essential libssl-dev valgrind
```

### Building and Testing

```bash
# Clone the repository
git clone <repository-url>
cd merkle-tree

# Run the test suite
cd tests
chmod +x run_tests.sh
./run_tests.sh

# Or build and test with make
make test

# Run with memory leak detection
./run_tests.sh --memory
```

### VS Code Development

The project includes complete VS Code configuration:

- **Build**: `Ctrl+Shift+B`
- **Run Tests**: `Ctrl+Shift+T`
- **Debug Tests**: `F5`
- **Memory Check**: `Ctrl+Shift+M`

## 📖 Usage

### Basic Example

```c
#include "Merkle.h"

int main() {
    // Sample data
    const char *data[] = {"Hello", "World", "Merkle", "Tree"};
    size_t sizes[] = {5, 5, 6, 4};
    size_t count = 4;
    size_t branching_factor = 2;  // Binary tree

    // Create Merkle tree
    merkle_tree_t *tree = merkle_tree_create(
        (const void **)data, sizes, count, branching_factor
    );

    if (tree) {
        printf("Merkle tree created successfully!\n");

        // Clean up
        merkle_tree_destroy(tree);
    } else {
        printf("Failed to create Merkle tree\n");
        return 1;
    }

    return 0;
}
```

### Advanced Configuration

```c
// Create a ternary Merkle tree (3 children per node)
merkle_tree_t *ternary_tree = merkle_tree_create(data, sizes, count, 3);

// Create a tree with large branching factor
merkle_tree_t *wide_tree = merkle_tree_create(data, sizes, count, 8);
```

### Error Handling

```c
merkle_tree_t *tree = merkle_tree_create(data, sizes, count, branching_factor);
if (!tree) {
    // Handle errors:
    // - NULL or invalid input parameters
    // - Memory allocation failures
    // - Tree construction failures
    printf("Tree creation failed\n");
    return -1;
}
```

## 🧪 Testing

The project includes a comprehensive test suite covering:

- ✅ **Input Validation**: NULL pointers, invalid sizes, edge cases
- ✅ **Functionality**: Tree creation, destruction, various configurations
- ✅ **Correctness**: Hash consistency and cryptographic properties
- ✅ **Memory Safety**: Leak detection and proper cleanup
- ✅ **Performance**: Large datasets and stress testing

### Running Tests

```bash
cd tests

# Basic test run
./run_tests.sh

# With verbose output
./run_tests.sh --verbose

# With memory leak detection
./run_tests.sh --memory

# With debug output
./run_tests.sh --debug
```

## 📚 Documentation

Complete API documentation is generated with Doxygen:

- **Browse**: Open [`docs/index.html`](docs/index.html) in your browser
- **Regenerate**: Run `doxygen Doxyfile` from project root

### Key API Functions

| Function                | Description                            |
| ----------------------- | -------------------------------------- |
| `merkle_tree_create()`  | Create a Merkle tree from data blocks  |
| `merkle_tree_destroy()` | Free all memory associated with a tree |

### Error Codes

| Code                       | Description                      |
| -------------------------- | -------------------------------- |
| `MERKLE_SUCCESS`           | Operation completed successfully |
| `MERKLE_NULL_ARG`          | NULL argument passed to function |
| `MERKLE_FAILED_MEM_ALLOC`  | Memory allocation failed         |
| `MERKLE_BAD_LEN`           | Invalid length or size argument  |
| `MERKLE_FAILED_TREE_BUILD` | Tree construction failed         |

## 🏗️ Algorithm Overview

The implementation uses a queue-based, bottom-up approach:

1. **Hash Generation**: Each data block is hashed using SHA-256
2. **Queue Processing**: Hashed blocks are placed in a processing queue
3. **Level Construction**: Nodes are grouped by branching factor and combined
4. **Tree Assembly**: Process continues until a single root node remains
5. **Root Hash**: The final root contains the cryptographic digest of all data

This approach efficiently handles any branching factor and automatically balances the tree structure.

## 🔧 Configuration

### Compile-Time Options

- `MERKLE_DEBUG`: Enable debug output for memory operations
- `HASH_SIZE`: SHA-256 hash size (32 bytes, not recommended to change)
- Custom branching factors supported at runtime

### Memory Management

The implementation uses custom memory management with optional debugging:

```c
#define MMalloc(x) Merkle_Malloc(x, __FILE__, __LINE__)
#define MFree(x) Merkle_Free(x, __FILE__, __LINE__)
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Run tests: `cd tests && ./run_tests.sh`
4. Ensure memory safety: `./run_tests.sh --memory`
5. Submit a pull request

## 🐛 Troubleshooting

### Build Issues

**OpenSSL not found:**

```bash
# Update OpenSSL path in Makefile
OPENSSL_ROOT = $(shell brew --prefix openssl@3)
```

**Deprecation warnings:**

```bash
# Already handled in Makefile with -Wno-deprecated-declarations
```

### Runtime Issues

**Segmentation faults:**

- Run with debug symbols: `make test-debug`
- Use memory checker: `./run_tests.sh --memory`
- Check for NULL input data or zero sizes

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
