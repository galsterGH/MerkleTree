# API Overview {#api_overview}

This page provides a comprehensive overview of the Merkle Tree Library API, including all public functions, data structures, and error codes.

## 📋 Table of Contents

- [Core Functions](#core-functions)
- [Data Structures](#data-structures)
- [Error Codes](#error-codes)
- [Memory Management](#memory-management)
- [Thread Safety](#thread-safety)
- [Performance Considerations](#performance-considerations)

## 🔧 Core Functions {#core-functions}

### Tree Creation and Destruction

#### create_merkle_tree()
```c
merkle_tree_t* create_merkle_tree(
    const void **data,
    const size_t *sizes,
    size_t count,
    size_t branching_factor
);
```

**Purpose**: Creates a new Merkle tree from an array of data blocks.

**Parameters**:
- `data`: Array of pointers to data blocks to include in the tree
- `sizes`: Array containing the size of each data block
- `count`: Number of data blocks (must be > 0)
- `branching_factor`: Number of children each internal node can have (must be > 0)

**Returns**: Pointer to the created tree, or `NULL` on failure.

**Thread Safety**: Not thread-safe. Only call from one thread.

**Example**:
```c
const char *data[] = {"block1", "block2", "block3"};
size_t sizes[] = {6, 6, 6};
merkle_tree_t *tree = create_merkle_tree(
    (const void**)data, sizes, 3, 2);
```

#### dealloc_merkle_tree()
```c
void dealloc_merkle_tree(merkle_tree_t *tree);
```

**Purpose**: Destroys a Merkle tree and frees all associated memory.

**Parameters**:
- `tree`: Pointer to the tree to destroy (can be NULL)

**Thread Safety**: Not thread-safe. Ensure no other threads are using the tree.

**Note**: After calling this function, the tree pointer becomes invalid.

### Hash Operations

#### get_tree_hash()
```c
merkle_error_t get_tree_hash(
    const merkle_tree_t *tree,
    unsigned char copy_into[HASH_SIZE]
);
```

**Purpose**: Retrieves the root hash of the Merkle tree.

**Parameters**:
- `tree`: Pointer to the tree (must not be NULL)
- `copy_into`: Buffer to store the 32-byte SHA-256 hash

**Returns**: `MERKLE_SUCCESS` on success, error code on failure.

**Thread Safety**: Thread-safe for concurrent reads.

**Example**:
```c
unsigned char root_hash[32];
if (get_tree_hash(tree, root_hash) == MERKLE_SUCCESS) {
    // Use root_hash...
}
```

### Proof Generation

#### generate_proof_from_index()
```c
merkle_error_t generate_proof_from_index(
    merkle_tree_t *const tree,
    size_t leaf_index,
    merkle_proof_t **proof
);
```

**Purpose**: Generates a Merkle proof for a leaf at the specified index.

**Parameters**:
- `tree`: Pointer to the tree (must not be NULL)
- `leaf_index`: Index of the leaf to generate proof for
- `proof`: Pointer to store the generated proof

**Returns**: `MERKLE_SUCCESS` on success, error code on failure.

**Thread Safety**: Thread-safe for concurrent reads.

**Note**: The caller is responsible for freeing the proof structure.

#### generate_proof_by_finder()
```c
merkle_error_t generate_proof_by_finder(
    merkle_tree_t *const tree,
    value_finder finder,
    size_t *path_length,
    merkle_proof_t** proof
);
```

**Purpose**: Generates a proof using a custom search function.

**Parameters**:
- `tree`: Pointer to the tree
- `finder`: Function pointer to locate the target value
- `path_length`: Pointer to store the proof path length
- `proof`: Pointer to store the generated proof

**Returns**: `MERKLE_SUCCESS` on success, error code on failure.

**Thread Safety**: Thread-safe for concurrent reads.

#### verify_proof()
```c
merkle_error_t verify_proof(
    const merkle_proof_t *proof,
    const unsigned char expected_root[HASH_SIZE],
    const void *leaf_data,
    size_t leaf_size
);
```

**Purpose**: Verifies a Merkle proof against expected root hash and leaf data.

**Parameters**:
- `proof`: The proof to verify
- `expected_root`: Expected root hash (32 bytes)
- `leaf_data`: Original leaf data
- `leaf_size`: Size of the leaf data

**Returns**: `MERKLE_SUCCESS` if valid, `MERKLE_PROOF_INVALID` if invalid.

**Thread Safety**: Thread-safe (stateless operation).

## 📊 Data Structures {#data-structures}

### merkle_tree_t
```c
typedef struct merkle_tree merkle_tree_t;
```
**Purpose**: Opaque structure representing a Merkle tree.

**Members**: Internal implementation - do not access directly.

**Usage**: Always use API functions to interact with tree structures.

### merkle_proof_t
```c
struct merkle_proof {
    merkle_proof_item_t **path;  // Array of proof items from leaf to root
    size_t path_length;          // Length of the proof path
    size_t leaf_index;           // Index of the leaf being proven
    size_t branching_factor;     // Branching factor of the tree
};
```

**Purpose**: Represents a complete Merkle proof for data verification.

**Members**:
- `path`: Array of proof items, one per tree level
- `path_length`: Number of levels in the proof path
- `leaf_index`: Original index of the proven leaf
- `branching_factor`: Tree's branching factor

### merkle_proof_item_t
```c
struct merkle_proof_item {
    unsigned char (*sibling_hashes)[HASH_SIZE]; // Arrays of sibling hashes
    size_t sibling_count;                       // Number of siblings
    size_t node_position;                       // Position among siblings
};
```

**Purpose**: Represents sibling information at one level of a Merkle proof.

**Members**:
- `sibling_hashes`: Array of sibling node hashes
- `sibling_count`: Number of sibling nodes at this level
- `node_position`: Position of our node among its siblings

### value_finder
```c
typedef bool (*value_finder)(const void *data);
```

**Purpose**: Function pointer type for custom leaf search operations.

**Parameters**:
- `data`: Pointer to leaf data to examine

**Returns**: `true` if this is the target leaf, `false` otherwise.

**Example**:
```c
bool find_transaction(const void *data) {
    const char *tx = (const char*)data;
    return strcmp(tx, "target_transaction") == 0;
}
```

## ⚠️ Error Codes {#error-codes}

### merkle_error_t
```c
typedef enum {
    MERKLE_SUCCESS = 0,           // Operation completed successfully
    MERKLE_NULL_ARG = -1,         // NULL argument provided
    MERKLE_BAD_ARG = -2,          // Invalid argument value
    MERKLE_BAD_LEN = -3,          // Invalid length parameter
    MERKLE_FAILED_MEM_ALLOC = -4, // Memory allocation failed
    MERKLE_FAILED_TREE_BUILD = -5,// Tree construction failed
    MERKLE_PROOF_INVALID = -6     // Proof verification failed
} merkle_error_t;
```

**Error Handling Pattern**:
```c
merkle_error_t result = some_function(...);
switch (result) {
    case MERKLE_SUCCESS:
        // Success - continue
        break;
    case MERKLE_NULL_ARG:
        // Handle null pointer error
        break;
    case MERKLE_FAILED_MEM_ALLOC:
        // Handle out of memory
        break;
    default:
        // Handle other errors
        break;
}
```

## 🧠 Memory Management {#memory-management}

### Allocation Patterns

The library follows these memory management principles:

1. **Tree Creation**: Memory is allocated during `create_merkle_tree()`
2. **Data Copying**: Input data is copied into the tree structure
3. **Cleanup**: All memory is freed by `dealloc_merkle_tree()`
4. **Proof Memory**: Caller responsible for freeing proof structures

### Memory Safety Features

- **Debug Mode**: Define `MERKLE_DEBUG` for allocation tracking
- **Null Pointer Safety**: All functions handle NULL inputs gracefully
- **Leak Detection**: Valgrind integration for testing

### Custom Memory Management

The library uses custom allocators internally:
```c
// Internal allocators (do not call directly)
void* MMalloc(size_t size);
void MFree(void* ptr);
```

## 🔒 Thread Safety {#thread-safety}

### Thread-Safe Operations

These operations can be called concurrently from multiple threads:

```c
// Read operations (thread-safe)
get_tree_hash(tree, hash);
generate_proof_from_index(tree, index, &proof);
generate_proof_by_finder(tree, finder, &length, &proof);
verify_proof(proof, root, data, size);
```

### Non-Thread-Safe Operations

These operations require exclusive access:

```c
// Write operations (not thread-safe)
merkle_tree_t *tree = create_merkle_tree(...);
dealloc_merkle_tree(tree);
```

### Locking Implementation

The library uses reader-writer locks internally:
- **Read operations** acquire shared locks
- **Write operations** acquire exclusive locks
- **Cross-platform** support (Windows SRWLOCK, POSIX pthread_rwlock)

## ⚡ Performance Considerations {#performance-considerations}

### Algorithm Complexity

| Operation | Time Complexity | Space Complexity |
|-----------|----------------|------------------|
| Tree Creation | O(n) | O(n) |
| Get Root Hash | O(1) | O(1) |
| Generate Proof | O(log n) | O(log n) |
| Verify Proof | O(log n) | O(1) |

### Memory Usage

- **Tree Storage**: ~40 bytes per node + data size
- **Proof Size**: ~32 × log_b(n) bytes (where b = branching factor)
- **Hash Cache**: 32 bytes per node for SHA-256 hashes

### Optimization Tips

1. **Branching Factor**: Higher values reduce tree depth but increase node size
2. **Data Size**: Smaller data blocks reduce memory usage
3. **Concurrent Reads**: Multiple threads can read simultaneously
4. **Memory Allocation**: Use memory pools for frequent tree creation/destruction

### Performance Testing

Benchmark the library with your specific use case:

```c
#include <time.h>

clock_t start = clock();
merkle_tree_t *tree = create_merkle_tree(data, sizes, count, bf);
clock_t end = clock();

double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
printf("Tree creation took %f seconds\\n", time_taken);
```

## 🔗 Constants

### Hash Size
```c
#define HASH_SIZE 32  // SHA-256 produces 32-byte hashes
```

### Build Configuration
```c
// Compile-time options
#define MERKLE_DEBUG     // Enable debug memory tracking
#define MERKLE_THREADED  // Enable thread safety (default)
```

---

For more detailed examples and use cases, see the @ref examples page.