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

/** Size in bytes of a SHA-256 hash digest. */
#define HASH_SIZE (32)

/** Identifier for data elements when building the tree. */
#define DATA_TYPE (1)

/** Identifier for hash elements when building the tree. */
#define HASH_TYPE (2)

/**
 * @brief Helper to call dealloc_queue_elements_variadic() with a terminator.
 */
#define DEALLOC_QUEUE_ELEMENTS(...)                                            \
  dealloc_queue_elements_variadic(__VA_ARGS__, (queue_element_t *)-1)


#define CLEAN_UP_NEXT_LVL(next_level,num_of_nodes)\
  do{\
    if(!(*next_level)) break;\
    for(size_t nd = 0; nd < num_of_nodes; ++nd){\
        if((*next_level)[nd]){\
          dealloc_hash_node((*next_level)[nd]);\
        }\
    }\
    MFree((*next_level));\
    *next_level = NULL;\
  }while(0)


/** True if there is only one element left in the queue. */
#define IS_LAST_ELEMENT(size) (size) == 1

#define IS_LEAF_NODES_LEVEL(tree_lvl) (tree_lvl) == 0

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
 * @brief Recursively deallocates a Merkle tree node and its children.
 * @param e Pointer to the Merkle node to deallocate.
 */
static void dealloc_hash_node(void *e);

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
        size_t branching_factor, merkle_tree_t **result);

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

static void dealloc_hash_node(void *e) {

  if (!e) {
    return;
  }

  merkle_node_t *node = e;

  if(node->children){
    for(size_t i = 0; i < node->child_count; ++i){
        dealloc_hash_node(node->children[i]);
    }
  
    MFree(node->children);
  }

  MFree(node);
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
       size_t branching_factor, merkle_tree_t **result) {

  size_t next_level_alloc_count = 0;
  merkle_node_t **next_level = NULL;

  if (!queue || !result) {
    return MERKLE_NULL_ARG;
  }

  if(branching_factor == 0){
    return MERKLE_FAILED_TREE_BUILD;
  }

  *result = MMalloc(sizeof **result);
  
  if (!(*result)) {
    return MERKLE_FAILED_MEM_ALLOC;
  }

  // zero out all the memory
  memset(*result ,0, sizeof **result); 

  /*
   * Process the queue level by level until a single node remains.
   * Each iteration collapses one tree level by combining up to
   * @p branching_factor child nodes under a newly allocated parent.
   */
  for (size_t tree_lvl = 0; get_queue_size(queue); tree_lvl++) {

    merkle_node_t *merkle_node = front_queue(queue);
    size_t queue_size = get_queue_size(queue);

    if(!merkle_node){
      MFree(*result);
      return MERKLE_FAILED_TREE_BUILD;
    }
      
    if (IS_LAST_ELEMENT(queue_size)) {

      if(pop_queue(queue) != merkle_node){
        MFree(*result);
        CLEAN_UP_NEXT_LVL((&next_level), next_level_alloc_count);
        return MERKLE_FAILED_TREE_BUILD;
      }

      (*result)->root = merkle_node;
      CLEAN_UP_NEXT_LVL((&next_level), next_level_alloc_count);
      return MERKLE_SUCCESS;
    }
      
    /* Number of parent nodes to create for this level. Each parent will
     * combine up to @p branching_factor children. */
    size_t full_nodes = (queue_size + branching_factor - 1)/branching_factor;

    if(IS_LEAF_NODES_LEVEL(tree_lvl)) {
      next_level = MMalloc(full_nodes *(sizeof(merkle_node_t *)));

      if(!next_level){
        MFree(*result);
        return MERKLE_FAILED_MEM_ALLOC;
      }

      next_level_alloc_count = full_nodes;
    }

    // clear all the pointers in next_level to be prepared to process the queue
    memset(next_level, 0, full_nodes * sizeof(merkle_node_t *));


    /* Build the next level of the tree by grouping children under new parents */
    for(size_t i = 0; i < full_nodes; ++i) {
      merkle_node_t *parent_node = MMalloc(sizeof *parent_node);

      if(!parent_node){
        MFree(*result);
        CLEAN_UP_NEXT_LVL((&next_level),next_level_alloc_count);
        return MERKLE_FAILED_MEM_ALLOC;
      }

      memset(parent_node,0,sizeof(*parent_node));
      next_level[i] = parent_node;

      /* Request up to branching_factor children from the queue.  The actual
       * number dequeued is returned in @p dequed. */
      size_t dequed = branching_factor;

      queue_result_t combo_result = deque_n(
        queue,
        &dequed,
        (void ***)&(parent_node->children));

      if(combo_result != QUEUE_OK){
        MFree(*result);
        CLEAN_UP_NEXT_LVL((&next_level),next_level_alloc_count);
        return MERKLE_FAILED_TREE_BUILD;
      }

      parent_node->child_count = dequed;

      /* Compute the parent's hash from its children. */
      merkle_error_t ret_code = hash_merkle_node(parent_node);

      if (ret_code != MERKLE_SUCCESS) {
        CLEAN_UP_NEXT_LVL((&next_level),next_level_alloc_count);
        MFree(*result);
        return ret_code;
      }
    }

    for(size_t i = 0; i < next_level_alloc_count; ++i){

      /* Enqueue the newly created parent for processing in the next level. */
      if(next_level[i] && push_queue(queue, next_level[i]) != QUEUE_OK){
        CLEAN_UP_NEXT_LVL((&next_level),next_level_alloc_count);
        MFree(*result);
        return MERKLE_FAILED_TREE_BUILD;
      }

      //next_level[i] should be NULLified since the ownership of the pointer has moved to the queue
      next_level[i] = NULL;
    }
  }

  CLEAN_UP_NEXT_LVL((&next_level),next_level_alloc_count);
  return MERKLE_SUCCESS;
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
      free_queue(queue, dealloc_hash_node);
      return NULL;
    }
    
    merkle_node_t *merkle_node = MMalloc(sizeof *merkle_node);

    if(!merkle_node){
      free_queue(queue, (dealloc_hash_node));
      return NULL;
    }

    if(hash_data_block(data[i], size[i],merkle_node->hash) != MERKLE_SUCCESS){
      dealloc_hash_node(merkle_node);
      free_queue(queue, dealloc_hash_node);
      return NULL;
    }

    if(push_queue(queue,merkle_node) != QUEUE_OK){
      dealloc_hash_node(merkle_node);
      free_queue(queue, dealloc_hash_node);
      return NULL;
    }
  }

  if (build_tree_from_queue(queue, branching_factor, &res) != MERKLE_SUCCESS) {
    res = NULL;
  }

  free_queue(queue, dealloc_hash_node);
  return res;
}
