/**
 * @file Merkle.h
 * @brief Public API for Merkle tree creation and destruction using SHA-256.
 *
 * This header defines the error codes, opaque types, and functions for
 * creating and destroying Merkle trees. The implementation uses SHA-256
 * and is suitable for cryptographic applications requiring data integrity
 * verification.
 *
 * @author Guy Alster
 * @date 2025-05-25
 */

#ifndef MERKLE_H
#define MERKLE_H

#include <stddef.h>

/** Size in bytes of a SHA-256 hash digest. */
#define HASH_SIZE (32)

/**
 * @enum merkle_error_t
 * @brief Error codes returned by Merkle tree operations.
 */
typedef enum merkle_error_t {
  MERKLE_SUCCESS = 0,      /**< Operation completed successfully. */
  MERKLE_NULL_ARG,         /**< Null argument passed to function. */
  MERKLE_FAILED_MEM_ALLOC, /**< Memory allocation failed. */
  MERKLE_BAD_LEN,          /**< Invalid length or size argument. */
  MERKLE_FAILED_TREE_BUILD /**< Merkle tree construction failed. */
} merkle_error_t;

/**
 * @struct merkle_tree
 * @brief Opaque structure representing a Merkle tree.
 */
struct merkle_tree;

/**
 * @typedef merkle_tree_t
 * @brief Typedef for the opaque Merkle tree structure.
 */
typedef struct merkle_tree merkle_tree_t;

/**
 * @brief Creates a Merkle tree from an array of data blocks.
 *
 * @param data   Array of pointers to data blocks.
 * @param sizes  Array of sizes for each data block.
 * @param count  Number of data blocks.
 * @param branching_factor Maximum number of children a node can have
 * @return Pointer to the created Merkle tree, or NULL on failure.
 */
merkle_tree_t *create_merkle_tree(const void **data, const size_t *sizes,
                                  size_t count, size_t branching_factor);

/**
 * @brief Destroys a Merkle tree and frees all associated memory.
 *
 * @param tree Pointer to the Merkle tree to destroy.
 */
void dealloc_merkle_tree(merkle_tree_t *tree);


/**
 * @brief Returns the Merkle Tree root's hash
 *
 * @param tree Pointer to the Merkle tree from which to get the root's hash
 */
 void get_tree_hash(merkle_tree_t *tree, unsigned char copy_into[HASH_SIZE]);

#endif // MERKLE_H
