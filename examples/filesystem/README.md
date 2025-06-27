# Filesystem Proof Example

This example demonstrates how to build a Merkle tree from files on disk and
verify the inclusion of a specific file. Three sample text files are provided in
the `data` directory. The program builds a binary Merkle tree from all files,
prints the resulting root hash, generates a proof for `file2.txt` and verifies
it.

## Building

Run the following command from this directory:

```bash
clang filesystem_example.c ../../src/merkle_tree.c ../../src/merkle_queue.c \
      ../../src/merkle_utils.c -I../../include -lssl -lcrypto -o fs_example
```

## Running

```
./fs_example
```

The program outputs the tree root hash and a verification message for
`file2.txt`.
