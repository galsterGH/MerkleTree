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
#include <stdbool.h>

/** Size in bytes of a SHA-256 hash digest. */
#define HASH_SIZE (32)

/**
 * @enum merkle_error_t
 * @brief Error codes returned by Merkle tree operations.
 */
typedef enum merkle_error_t {
  MERKLE_SUCCESS = 0,      /**< Operation completed successfully. */
  MERKLE_NULL_ARG,         /**< Null argument passed to function. */
  MERKLE_BAD_ARG,          /**< Invalid argument value provided. */
  MERKLE_FAILED_MEM_ALLOC, /**< Memory allocation failed. */
  MERKLE_BAD_LEN,          /**< Invalid length or size argument. */
  MERKLE_FAILED_TREE_BUILD,/**< Merkle tree construction failed. */
  MERKLE_INVALID_INDEX,    /**< Leaf index out of bounds. */
  MERKLE_PROOF_INVALID,    /**< Proof verification failed. */
  MERKLE_NOT_FOUND         /**< Target value not found in tree. */
} merkle_error_t;

/**
 * @typedef value_finder
 * @brief Function pointer type for finding specific values in tree leaves.
 * 
 * @param value Pointer to the value to evaluate
 * @return true if the value matches the search criteria, false otherwise
 */
typedef bool (*value_finder)(void *value);

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


struct merkle_proof_item;
typedef struct merkle_proof_item merkle_proof_item_t;

struct merkle_proof;
typedef struct merkle_proof merkle_proof_t;

/**
 * @brief Creates a Merkle tree from an array of data blocks.
 *
 * Constructs a Merkle tree using SHA-256 hashing with the specified branching factor.
 * The branching factor must be >= 2 and affects the tree's structure and height.
 *
 * @param data Array of pointers to data blocks (must not be NULL).
 * @param sizes Array of sizes for each data block (must not be NULL).
 * @param count Number of data blocks (must be > 0).
 * @param branching_factor Maximum number of children per node (must be >= 2).
 * @return Pointer to the created Merkle tree, or NULL on failure.
 */
merkle_tree_t *create_merkle_tree(const void **data, const size_t *sizes,
                                  size_t count, size_t branching_factor);

/**
 * @brief Destroys a Merkle tree and frees all associated memory.
 *
 * This function safely handles NULL pointers and will not crash if
 * a NULL tree pointer is passed.
 *
 * @param tree Pointer to the Merkle tree to destroy (can be NULL).
 */
void dealloc_merkle_tree(merkle_tree_t *tree);


/**
 * @brief Returns the Merkle Tree root's hash.
 *
 * Copies the root hash of the Merkle tree into the provided buffer.
 * The buffer must be at least HASH_SIZE (32) bytes in size.
 *
 * @param tree Pointer to the Merkle tree (must not be NULL).
 * @param copy_into Buffer to copy the hash into (must be HASH_SIZE bytes).
 * @return MERKLE_SUCCESS on success, MERKLE_NULL_ARG if either parameter is NULL.
 */
merkle_error_t get_tree_hash(const merkle_tree_t *tree, unsigned char copy_into[HASH_SIZE]);


/**
 * @brief Generates a Merkle proof for a leaf at the specified index.
 *
 * Creates a proof path from the specified leaf to the root. The proof can be used
 * to verify that the leaf is part of the tree without reconstructing the entire tree.
 * Returns NULL if the leaf index is out of bounds or if memory allocation fails.
 *
 * @param tree Pointer to the Merkle tree (must not be NULL).
 * @param leaf_index Index of the leaf to generate proof for (must be < leaf count).
 * @param proof Pointer to store the generated proof (must not be NULL).
 * @param path_length Pointer to store the length of the proof path (must not be NULL).
 * @return MERKLE_SUCCESS on success, error code on failure.
 */
merkle_error_t generate_proof_from_index(const merkle_tree_t *tree, size_t leaf_index, merkle_proof_t **proof);

/**
 * @brief Generates a Merkle proof for a leaf found using a custom finder function.
 *
 * Searches the tree for a leaf matching the criteria defined by the finder function,
 * then generates a proof path for the first matching leaf found. The search is
 * performed in leaf index order (0 to leaf_count-1).
 *
 * @param tree Pointer to the Merkle tree (must not be NULL).
 * @param finder Function pointer to locate the target leaf value (must not be NULL).
 * @param path_length Pointer to store the length of the proof path (must not be NULL).
 * @return Pointer to the generated proof on success, NULL on failure (null arguments,
 *         no matching leaf found, or memory allocation failure).
 */
merkle_error_t generate_proof_by_finder(const merkle_tree_t *tree, value_finder finder, size_t *path_length, merkle_proof_t** proof);

/**
 * @brief Verify a Merkle proof against a known root hash.
 *
 * Recomputes the hash chain from the supplied leaf data using the proof path
 * and compares the resulting root hash to @p expected_root.
 *
 * @param proof         Pointer to the proof to verify.
 * @param expected_root Expected root hash of the tree.
 * @param leaf_data     Data contained in the proved leaf.
 * @param leaf_size     Size of @p leaf_data in bytes.
 * @return MERKLE_SUCCESS if the proof matches the root, MERKLE_PROOF_INVALID on
 *         mismatch or an error code on invalid arguments.
 */
merkle_error_t verify_proof(const merkle_proof_t *proof,
                            const unsigned char expected_root[HASH_SIZE],
                            const void *leaf_data, size_t leaf_size);


#endif // MERKLE_H
