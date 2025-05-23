# Merkle Tree Implementation

This project provides an implementation of a Merkle Tree in C. A Merkle Tree is a cryptographic data structure used to verify the integrity and consistency of data efficiently. It is widely used in blockchain technologies, distributed systems, and cryptographic applications.

## Features
- Implementation of a queue data structure for managing tree nodes.
- Support for hashing data blocks using SHA-256.
- Construction of a Merkle Tree from input data.
- Memory management utilities for efficient allocation and deallocation.

## Files
- `Merkle.c`: Contains the implementation of the Merkle Tree and related functions.
- `Merkle.h`: Header file for the Merkle Tree implementation.
- `Utils.c`: Contains the implementation of a queue data structure and utility functions.
- `Utils.h`: Header file for the utility functions and queue implementation.

## Build Instructions
To build the project, ensure you have OpenSSL installed and run the following command:

```bash
clang -g Merkle.c Utils.c -o merkle \
    -I/opt/homebrew/opt/openssl@3/include \
    -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto
```

Alternatively, you can use the provided build task in VS Code.

## Usage
The `merkle_tree_create` function can be used to create a Merkle Tree from an array of data blocks. Example usage:

```c
#include "Merkle.h"

const void *data[] = {"block1", "block2", "block3"};
const size_t sizes[] = {6, 6, 6};
unsigned int count = 3;

struct MerkleTree *tree = merkle_tree_create(data, sizes, count);
if (tree) {
    // Use the tree
    merkle_free_destroy(tree);
}
```

## License
This project is licensed under the MIT License. See the LICENSE file for details.
