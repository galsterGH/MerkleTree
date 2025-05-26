/**
 * @file Merkle.c
 * @brief Implementation of a Merkle tree using SHA-256 and a queue-based
 * approach.
 *
 * This file provides the implementation for creating, destroying, and managing
 * Merkle trees. It includes utility functions for hashing, memory management,
 * and queue operations, as well as internal data structures for Merkle nodes,
 * proofs, and queue elements.
 *
 * @author Guy Alster
 * @date 2025-05-25
 */

#include <openssl/sha.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Merkle.h"
#include "MerkleQueue.h"
#include "Utils.h"

#define HASH_SIZE (32)
#define DATA_TYPE (1)
#define HASH_TYPE (2)

#define FREE_ALL(...) free_all_variadic(__VA_ARGS__, (void *)-1)
#define DEALLOC_QUEUE_ELEMENTS(...)                                            \
  dealloc_queue_elements_variadic(__VA_ARGS__, (queue_element_t *)-1)


#define HASH_TWO_NODES_SHA256(n1, n2, out)         \
  do {                                             \
    SHA256_CTX ctx;                                \
    SHA256_Init(&ctx);                             \
    SHA256_Update(&ctx, (n1)->hash, HASH_SIZE);    \
    SHA256_Update(&ctx, (n2)->hash, HASH_SIZE);    \
    SHA256_Final((out), &ctx);                     \
  } while (0)

/**
 * @brief Policy for deallocation of queue elements.
 */
typedef enum deallocation_policy {
  Dont_Dealloc = 0, /**< Do not deallocate. */
  Dealloc = 1       /**< Deallocate memory. */
} deallocation_policy;

/**
 * @brief Represents a node in the Merkle tree.
 */
typedef struct merkle_node {
  unsigned char hash[HASH_SIZE];    /**< SHA-256 hash of the node. */
  struct merkle_node *left, *right; /**< Pointers to left and right children. */
} merkle_node_t;

/**
 * @brief Represents an item in a Merkle proof.
 */
typedef struct merkle_proof_item {
  unsigned char hash[HASH_SIZE]; /**< Sibling hash. */
  unsigned char is_left; /**< 1 if sibling is on the left, 0 if on the right. */
} merkle_proof_item_t;

/**
 * @brief Represents a Merkle tree.
 */
struct merkle_tree {
  merkle_node_t *root;    /**< Root node of the tree. */
  merkle_node_t **leaves; /**< Array of leaf nodes. */
  size_t leaf_count;      /**< Number of leaves. */
};

/**
 * @brief Represents an element in the queue used for tree construction.
 */
typedef struct queue_element_t {
  union {
    struct {
      void *d;     /**< Pointer to data. */
      size_t size; /**< Size of the data. */
    } data;
    merkle_node_t *mnode; /**< Pointer to a Merkle node. */
  };
  int queue_element_type; /**< Type of the queue element (DATA_TYPE or
                             HASH_TYPE). */
} queue_element_t;

/**
 * @brief Frees a variadic list of pointers using MFree.
 * @param first The first pointer to free.
 * @param ... Additional pointers to free, terminated by (void *)-1.
 */
static void free_all_variadic(void *first, ...);

/**
 * @brief Returns a deallocator function pointer based on the policy.
 * @param policy The deallocation policy.
 * @return The deallocator function pointer or NULL.
 */
static deallocator get_queue_dealloc(deallocation_policy policy);

/**
 * @brief Recursively deallocates a Merkle tree node and its children.
 * @param e Pointer to the Merkle node to deallocate.
 */
static void dealloc_hash_node(merkle_node_t *e);

/**
 * @brief Deallocates a queue element, including its data or Merkle node.
 * @param e Pointer to the queue element to deallocate.
 */
static void dealloc_queue_element(void *e);

/**
 * @brief Deallocates a variadic list of queue elements.
 * @param first The first queue element to deallocate.
 * @param ... Additional queue elements, terminated by (queue_element_t *)-1.
 */
static void dealloc_queue_elements_variadic(queue_element_t *first, ...);

/**
 * @brief Hashes a data block using SHA-256.
 * @param data Pointer to the data.
 * @param size Size of the data.
 * @param out Output buffer for the hash (must be HASH_SIZE bytes).
 * @return MERKLE_SUCCESS on success, error code otherwise.
 */
static merkle_error_t hash_data_block(const void *data, size_t size,
                                      unsigned char out[HASH_SIZE]);

/**
 * @brief Hashes two Merkle nodes together using SHA-256.
 * @param n1 Pointer to the first node.
 * @param n2 Pointer to the second node.
 * @param out Output buffer for the hash (must be HASH_SIZE bytes).
 * @return MERKLE_SUCCESS on success, error code otherwise.
 */
static merkle_error_t hash_two_nodes(merkle_node_t *n1, merkle_node_t *n2,
                                     unsigned char out[HASH_SIZE]);

/**
 * @brief Builds a Merkle tree from a queue of elements.
 * @param queue Pointer to the queue.
 * @param result Output pointer to the resulting Merkle tree.
 * @return MERKLE_SUCCESS on success, error code otherwise.
 */
static merkle_error_t build_tree_from_queue(queue_t *queue,
                                            merkle_tree_t **result);

/**
 * @brief Destroys a Merkle tree and frees all associated memory.
 * @param tree Pointer to the Merkle tree to destroy.
 */
void merkle_tree_destroy(merkle_tree_t *tree);

/**
 * @brief Creates a Merkle tree from an array of data blocks.
 * @param data Array of pointers to data blocks.
 * @param size Array of sizes for each data block.
 * @param count Number of data blocks.
 * @return Pointer to the created Merkle tree, or NULL on failure.
 */
merkle_tree_t *merkle_tree_create(const void **data, const size_t *size,
                                  size_t count);

// --- Implementation of the above functions follows ---

static void free_all_variadic(void *first, ...) {
  va_list args;
  va_start(args, first);
  void *ptr = first;

  while (ptr != (void *)-1) {
  
    if (ptr != NULL) {
      MFree(ptr);
    }
  
    ptr = va_arg(args, void *);
  }
  
  va_end(args);
}

static deallocator get_queue_dealloc(deallocation_policy policy) {

  if (policy == Dealloc) {
    return &free;
  }

  return NULL;
}

static void dealloc_hash_node(merkle_node_t *e) {
  
  if (!e) {
    return;
  }

  dealloc_hash_node(e->left);
  dealloc_hash_node(e->right);
  MFree(e);
}

static void dealloc_queue_element(void *e) {

  queue_element_t *qe = e;
  
  if (!qe) {
    return;
  }

  if (qe->queue_element_type == DATA_TYPE) {
    FREE_ALL(qe->data.d, qe);
    return;
  }

  dealloc_hash_node(qe->mnode);
}

static void dealloc_queue_elements_variadic(queue_element_t *first, ...) {
  va_list args;
  va_start(args, first);

  while (first != (queue_element_t *)-1) {
    if (first != NULL) {
      dealloc_queue_element(first);
    }

    first = va_arg(args, queue_element_t *);
  }

  va_end(args);
}

static merkle_error_t hash_data_block(const void *data, size_t size,
                                      unsigned char out[HASH_SIZE]) {
  if (!data || !out) {
    return MERKLE_NULL_ARG;
  }

  if (size == 0) {
    return MERKLE_BAD_LEN;
  }

  SHA256((const unsigned char *)data, size, out);
  return MERKLE_SUCCESS;
}

static merkle_error_t hash_two_nodes(merkle_node_t *n1, merkle_node_t *n2,
                                     unsigned char out[HASH_SIZE]) {
  if (!n1 || !n2 || !out) {
    return MERKLE_NULL_ARG;
  }

  HASH_TWO_NODES_SHA256(n1, n2, out);
  return MERKLE_SUCCESS;
}

static merkle_error_t build_tree_from_queue(queue_t *queue,
                                            merkle_tree_t **result) {
  if (!queue || !result) {
    return MERKLE_NULL_ARG;
  }

  merkle_error_t ret_code = MERKLE_SUCCESS;
  *result = MMalloc(sizeof *result);
  
  if (!(*result)) {
    ret_code = MERKLE_FAILED_MEM_ALLOC;
    goto failure;
  }

  while (get_queue_size(queue)) {

    queue_element_t *element = pop_queue(queue);
    
    if (element->queue_element_type == DATA_TYPE) {
      queue_element_t *h_el = MMalloc(sizeof *h_el);
    
      if (!h_el) {
        DEALLOC_QUEUE_ELEMENTS(element);
        ret_code = MERKLE_FAILED_MEM_ALLOC;
        goto failure;
      }

      h_el->queue_element_type = HASH_TYPE;
      h_el->mnode = MMalloc(sizeof *(h_el->mnode));
      ret_code = hash_data_block(element->data.d, element->data.size,
                                 (h_el->mnode->hash));

      if (ret_code != MERKLE_SUCCESS) {
        DEALLOC_QUEUE_ELEMENTS(h_el, element);
        goto failure;
      }

      if (push_queue(queue, h_el) != QUEUE_OK) {
        DEALLOC_QUEUE_ELEMENTS(h_el, element);
        ret_code = MERKLE_FAILED_TREE_BUILD;
        goto failure;
      }

    } else {

      if (!get_queue_size(queue)) {
        (*result)->root = element->mnode;
      } else {
        queue_element_t *next_element = pop_queue(queue);
        queue_element_t *combined = MMalloc(sizeof *combined);

        if (!combined) {
          DEALLOC_QUEUE_ELEMENTS(element, next_element);
          ret_code = MERKLE_FAILED_MEM_ALLOC;
          goto failure;
        }

        combined->queue_element_type = HASH_TYPE;
        combined->mnode = MMalloc(sizeof *(combined->mnode));
        unsigned char out_hash[HASH_SIZE];

        ret_code =
            hash_two_nodes(element->mnode, next_element->mnode, &(out_hash[0]));

        if (ret_code != MERKLE_SUCCESS) {
          DEALLOC_QUEUE_ELEMENTS(element, next_element, combined);
          goto failure;
        }

        memcpy(&(combined->mnode->hash[0]), &(out_hash[0]), HASH_SIZE);
        combined->mnode->left = element->mnode;
        combined->mnode->right = next_element->mnode;

        // clean up the queue nodes
        element->mnode = NULL;
        next_element->mnode = NULL;
        DEALLOC_QUEUE_ELEMENTS(element, next_element);
        push_queue(queue, combined);
      }
    }
  }
  goto success;
failure:
  MFree(result);
success:
  return ret_code;
}

void merkle_tree_destroy(merkle_tree_t *tree) {
  if (!tree){
    return;
  }

  dealloc_hash_node(tree->root);
  MFree(tree->leaves); // if you store this later
  MFree(tree);
}

merkle_tree_t *merkle_tree_create(const void **data, const size_t *size,
                                  size_t count) {
  merkle_tree_t *res = NULL;

  if (!(data && *data && size && *size > 0 && count > 0)) {
    return NULL;
  }

  queue_t *queue = init_queue();

  for (int i = 0; i < count; ++i) {
    if (!data[i] || !size[i]) {
      free_queue(queue, dealloc_queue_element);
      return NULL;
    }

    queue_element_t *element = MMalloc(sizeof *element);
    if (!element) {
      goto cleanup;
    }

    element->data.d = data[i];
    element->data.size = size[i];
    element->queue_element_type = DATA_TYPE;
    push_queue(queue, element);
  }

  // for odd queue size we should duplicate the last element
  if (get_queue_size(queue) % 2) {
    queue_element_t *element = MMalloc(sizeof *element);
    
    if (!element) {
      goto cleanup;
    }

    element->data.d = MMalloc(size[count - 1]);
    
    if (!(element->data.d)) {
      dealloc_queue_element(element);
      goto cleanup;
    }
    
    memcpy(element->data.d, data[count - 1], size[count - 1]);
    element->data.size = size[count - 1];
    element->queue_element_type = DATA_TYPE;
    push_queue(queue, element);
  }

  if (build_tree_from_queue(queue, &res) != MERKLE_SUCCESS) {
    res = NULL;
  }
  
cleanup:
  free_queue(queue, dealloc_queue_element);
  return res;
}

/**
 * @brief Main entry point for testing.
 */
int main() { return 0; }
