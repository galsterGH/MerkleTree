# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Test Commands

### Quick Start
```bash
# Most common workflow - run tests with the enhanced test runner
cd tests && ./run_tests.sh

# Run with memory leak detection (preferred for development)
cd tests && ./run_tests.sh --memory
```

### Building Tests
```bash
cd tests
make all              # Build test executable
make clean            # Clean build artifacts
make rebuild          # Clean and rebuild
make help             # Show all available targets
```

### Running Tests
```bash
cd tests
make test             # Run basic tests
make test-memory      # Run with memory leak detection (requires valgrind)
make test-debug       # Run with debug output enabled

# Enhanced test runner (recommended)
./run_tests.sh        # Colored output with build status
./run_tests.sh --memory   # Memory leak detection
./run_tests.sh --debug    # Debug mode with MERKLE_DEBUG
./run_tests.sh --verbose  # Show full build output
./run_tests.sh --help     # Show all options
```

### Building Main Executable
```bash
# No main executable - this is a library project
# Integration requires linking against the compiled objects
```

## Development Guidelines

### Memory Management
**CRITICAL**: Always use custom memory management macros:
- `MMalloc(size)` - Debug-aware allocation with file/line tracking
- `MFree(ptr)` - Debug-aware deallocation with file/line tracking

Never use `malloc()`/`free()` directly. These macros enable memory debugging when `MERKLE_DEBUG` is defined.

### Code Style
- Follow existing patterns in the codebase
- Use cleanup macros: `CLEAN_UP_TREE()`, `CLEAN_UP_NEXT_LVL()`
- Check all function return values, especially for memory allocation
- Maintain fail-fast error handling pattern

### Common Pitfalls
- **API inconsistency**: Public API uses `create_merkle_tree()` but internal functions may use different naming
- **Incomplete features**: Merkle proof generation functions are partially implemented
- **OpenSSL path**: Makefile hardcodes OpenSSL path - may need adjustment for different systems

## Architecture Overview

This is a **C implementation of n-ary Merkle Trees** with the following key components:

### Core Structure (src/)
- **merkle_tree.c**: Main implementation with queue-based bottom-up tree construction
- **merkle_queue.c**: Custom queue data structure for tree building process  
- **merkle_utils.c**: Memory management utilities with debug tracking

### Public API (include/)
- **Merkle.h**: Main public API for tree creation/destruction and proof generation
- **MerkleQueue.h**: Queue interface used internally for tree construction
- **Utils.h**: Memory management macros and compiler optimization hints

### Key Data Structures
- `merkle_tree_t`: Opaque tree structure containing root, leaves array, and metadata
- `merkle_node_t`: Internal node structure with hash, children array, parent pointers, and data
- `merkle_proof_t` & `merkle_proof_item_t`: Proof structures (partially implemented)

### Tree Construction Algorithm
1. Hash input data blocks using SHA-256
2. Create leaf nodes and add to processing queue
3. Use queue-based level-by-level construction:
   - Group nodes by branching factor
   - Create parent nodes combining children hashes
   - Continue until single root remains
4. Maintains parent-child relationships and node indexing

### Key Features
- **N-ary trees**: Configurable branching factor (binary, ternary, etc.)
- **SHA-256 hashing**: Uses OpenSSL for cryptographic operations
- **Memory safety**: Custom allocation tracking with `MERKLE_DEBUG` support
- **Queue-based construction**: Efficient bottom-up tree building
- **Opaque API**: Clean separation of public interface and implementation

### Dependencies
- **OpenSSL 3.x**: For SHA-256 hashing operations
- **clang/gcc**: C11 compiler support required
- **valgrind** (optional): For memory leak detection during testing

### Important Implementation Details
- **Fail-fast design**: Tree construction fails immediately on any memory allocation error
- **Bidirectional links**: All nodes maintain parent-child relationships with indexing
- **Data copying**: Leaf nodes store copies of original data (not references)
- **Level tracking**: Tree levels counter tracks depth for proof path calculations
- **Error handling**: Custom cleanup macros ensure proper memory deallocation on errors
- **Queue-based algorithm**: Uses custom queue implementation with sentinel nodes

## Known Issues & Limitations

- **Incomplete proof generation**: `generate_proof_from_index()` function is not fully implemented
- **Platform dependency**: OpenSSL path in Makefile assumes macOS Homebrew installation
- **No proof verification**: Missing functions to verify generated proofs
- **Limited error context**: Error codes don't provide detailed failure context

## File Locations Reference

```
src/merkle_tree.c:441-462    # Incomplete proof generation implementation
include/Merkle.h:69          # Typo in comment ("@brief Destroys" has extra 's')
tests/Makefile:9             # Hardcoded OpenSSL path
src/merkle_tree.c:447        # Variable name collision in proof generation
```