/**
 * @file test_merkle_internal.h
 * @brief Internal structures for testing purposes only.
 *
 * This header exposes internal structures that are normally opaque.
 * FOR TESTING USE ONLY - DO NOT USE IN PRODUCTION CODE.
 */

#ifndef TEST_MERKLE_INTERNAL_H
#define TEST_MERKLE_INTERNAL_H

#include "../include/Merkle.h"

#define HASH_SIZE (32)

/**
 * @brief Internal structure of a Merkle tree node (for testing).
 */
typedef struct merkle_node {
  unsigned char hash[HASH_SIZE];
  struct merkle_node **children;
  size_t child_count;
} merkle_node_t;

/**
 * @brief Internal structure of a Merkle tree (for testing).
 */
struct merkle_tree {
  merkle_node_t *root;
  merkle_node_t **leaves;
  size_t leaf_count;
};

#endif // TEST_MERKLE_INTERNAL_H