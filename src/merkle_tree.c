/**
 * @file merkle_tree.c
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

#define DEALLOC_QUEUE_ELEMENTS(...)                                            \
  dealloc_queue_elements_variadic(__VA_ARGS__, (queue_element_t *)-1)

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
    unsigned char hash[HASH_SIZE];
    struct merkle_node **children;
    size_t child_count;
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
 * @brief Hashes a Merkle node by combining the hashes of its children.
 * @param parent Pointer to the Merkle node to hash.
 * @return MERKLE_SUCCESS on success, error code otherwise.
 **/
static merkle_error_t hash_merkle_node(merkle_node_t *parent);

/**
 * @brief Builds a Merkle tree from a queue of elements.
 * @param queue Pointer to the queue.
 * @param result Output pointer to the resulting Merkle tree.
 * @return MERKLE_SUCCESS on success, error code otherwise.
 */
static merkle_error_t build_tree_from_queue(queue_t *queue,
        const size_t branching_factor, merkle_tree_t **result);

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
                                  size_t count, size_t branching_factor);

static void dealloc_hash_node(merkle_node_t *e) {
  
  if (!e) {
    return;
  }

  if(e->children){
    for(size_t i = 0; i < e->child_count; ++i){
        dealloc_hash_node(e->children[i]);
    }
  
    MFree(e->children);
  }

  MFree(e);
}

static void dealloc_queue_element(void *e) {

  queue_element_t *qe = e;
  
  if (!qe) {
    return;
  }

  if (qe->queue_element_type == DATA_TYPE) {
    MFree(qe);
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


static merkle_error_t hash_merkle_node(merkle_node_t *parent){
  if (!parent) {
    return MERKLE_NULL_ARG;
  }

  if(parent->child_count == 0 || !parent->children) {
    return MERKLE_SUCCESS; // No children to hash
  }

  // Initialize SHA-256 context
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  
  for(size_t i = 0; i < parent->child_count; ++i){
    if(!parent->children[i]){
      return MERKLE_NULL_ARG;
    }

    SHA256_Update(&ctx, parent->children[i]->hash,HASH_SIZE);      
  }
   
  //finalize
  SHA256_Final(parent->hash,&ctx);
  return MERKLE_SUCCESS;
}

static merkle_error_t build_tree_from_queue(queue_t *queue,
      const size_t branching_factor, merkle_tree_t **result) {

  if (!queue || !result) {
    return MERKLE_NULL_ARG;
  }

  if(branching_factor == 0){
    return MERKLE_FAILED_TREE_BUILD;
  }

  merkle_error_t ret_code = MERKLE_SUCCESS;
  *result = MMalloc(sizeof **result);
  
  if (!(*result)) {
    ret_code = MERKLE_FAILED_MEM_ALLOC;
    goto failure;
  }

  // zero out all the memory
  memset(*result ,0, sizeof **result); 

  while (get_queue_size(queue)) {

    queue_element_t *element = front_queue(queue);

    if(!element){
      ret_code = MERKLE_FAILED_TREE_BUILD;
      goto failure;
    }
    
    if (element->queue_element_type == DATA_TYPE) {
      queue_element_t *h_el = MMalloc(sizeof *h_el);
    
      if (!h_el) {
        ret_code = MERKLE_FAILED_MEM_ALLOC;
        goto failure;
      }

      h_el->queue_element_type = HASH_TYPE;
      h_el->mnode = MMalloc(sizeof *(h_el->mnode));
      memset(h_el->mnode, 0, sizeof(*(h_el->mnode)));

      ret_code = hash_data_block(element->data.d, element->data.size,
                                 (h_el->mnode->hash));

      if (ret_code != MERKLE_SUCCESS) {
        DEALLOC_QUEUE_ELEMENTS(h_el);
        goto failure;
      }

      if (push_queue(queue, h_el) != QUEUE_OK) {
        DEALLOC_QUEUE_ELEMENTS(h_el);
        ret_code = MERKLE_FAILED_TREE_BUILD;
        goto failure;
      }

      // only pop the queue at the end 
      pop_queue(queue);

    } else {
      
      size_t queue_size = get_queue_size(queue);

      if (queue_size == 1) {
        (*result)->root = element->mnode;
        pop_queue(queue);

      } else {

        size_t full_nodes = (queue_size + branching_factor - 1)/branching_factor;
      
        for(size_t i = 0; i < full_nodes; ++i) {
          queue_element_t *combined = MMalloc(sizeof *combined);
          
          if (!combined) {
            ret_code = MERKLE_FAILED_MEM_ALLOC;
            goto failure;
          }

          combined->queue_element_type = HASH_TYPE;
          combined->mnode = MMalloc(sizeof *(combined->mnode));

          if(!combined->mnode) {
            DEALLOC_QUEUE_ELEMENTS(combined);
            ret_code = MERKLE_FAILED_MEM_ALLOC;
            goto failure;
          }

          size_t dequed = branching_factor;

          queue_result_t combo_result = deque_n(
            queue, 
            &dequed, 
            (void ***)&(combined->mnode->children));
          
          if(combo_result != QUEUE_OK){
            DEALLOC_QUEUE_ELEMENTS(combined);
            ret_code = MERKLE_FAILED_TREE_BUILD;
            goto failure;
          }

          combined->mnode->child_count = dequed;
          ret_code = hash_merkle_node(combined->mnode);

          if (ret_code != MERKLE_SUCCESS) {
            DEALLOC_QUEUE_ELEMENTS(combined);
            goto failure;
          }

          if(push_queue(queue, combined) != QUEUE_OK){
            DEALLOC_QUEUE_ELEMENTS(combined);
            ret_code = MERKLE_FAILED_TREE_BUILD;
            goto failure;
          }
        }
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
                                  size_t count, size_t branching_factor) {
  merkle_tree_t *res = NULL;

  if (!(data && count > 0 && branching_factor > 0)) {
    return NULL;
  }

  queue_t *queue = init_queue();

  if(!queue){
    return NULL;
  }

  for (size_t i = 0; i < count; ++i) {

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

  if (build_tree_from_queue(queue,branching_factor, &res) != MERKLE_SUCCESS) {
    res = NULL;
  }

cleanup:
  free_queue(queue, dealloc_queue_element);
  return res;
}