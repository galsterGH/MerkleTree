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
#include <stdbool.h>

#include "Merkle.h"
#include "MerkleQueue.h"
#include "merkle_utils.h"
#include "locking.h"


/** True if there is only one element left in the queue. */
#define IS_LAST_ELEMENT(size) (size) == 1

/** True if we're processing the leaf nodes level (level 0). */
#define IS_LEAF_NODES_LEVEL(tree_lvl) (tree_lvl) == 0

/**
 * @brief Represents a node in the Merkle tree.
 */
typedef struct merkle_node {
    unsigned char hash[HASH_SIZE];   /**< SHA-256 hash of this node. */
    void *data;                      /**< Data stored in leaf nodes (NULL for internal nodes). */
    struct merkle_node **children;   /**< Array of child node pointers. */
    struct merkle_node *parent;      /**< Pointer to parent node (NULL for root). */
    size_t index_in_parent;          /**< Index of this node in parent's children array. */
    size_t child_count;              /**< Number of children this node has. */
} merkle_node_t;

/**
 * @brief Represents a Merkle tree.
 */
struct merkle_tree {
  rw_lock_t lock;
  merkle_node_t *root;    /**< Root node of the tree. */
  merkle_node_t **leaves; /**< Array of leaf nodes. */
  size_t leaf_count;      /**< Number of leaves. */
  size_t levels;          /**< Number of levels in the tree. */
  size_t branching_factor;
};

/**
 * @brief Represents a single item in a Merkle proof path.
 */
struct merkle_proof_item {
    unsigned char (*sibling_hashes)[HASH_SIZE]; /**< Arrays of sibling hashes at this level. */
    size_t sibling_count;           /**< Number of siblings at this level. */
    size_t node_position;           /**< Position of our node among siblings. */
};

/**
 * @brief Represents a complete Merkle proof.
 */
struct merkle_proof{
    merkle_proof_item_t **path;  /**< Array of proof items from leaf to root. */
    size_t path_length;          /**< Length of the proof path. */
    size_t leaf_index;           /**< Index of the leaf being proven. */
    size_t branching_factor;     /**< Branching factor of the tree. */
};

/**
 * @brief Destroys a Merkle tree and frees all associated memory.
 * @param tree Pointer to the Merkle tree to destroy.
 */
void dealloc_merkle_tree(merkle_tree_t *tree);

/**
 * @brief Creates a Merkle tree from an array of data blocks.
 * @param data Array of pointers to data blocks.
 * @param size Array of sizes for each data block.
 * @param count Number of data blocks.
 * @return Pointer to the created Merkle tree, or NULL on failure.
 */
merkle_tree_t *create_merkle_tree(const void **data, const size_t *size,
                                  size_t count, size_t branching_factor);


/**
 * @brief Returns the Merkle Tree root's hash.
 * @param tree Pointer to the Merkle tree (must not be NULL).
 * @param copy_into Buffer to copy the hash into (must be HASH_SIZE bytes).
 * @return MERKLE_SUCCESS on success, error code on failure.
 */
merkle_error_t get_tree_hash(merkle_tree_t * const tree, unsigned char copy_into[HASH_SIZE]);

/**
 * @brief Generates a Merkle proof for a leaf at the specified index.
 * @param tree Pointer to the Merkle tree (must not be NULL).
 * @param leaf_index Index of the leaf to generate proof for.
 * @param proof Pointer to store the generated proof.
 * @param path_length Pointer to store the length of the proof path.
 * @return MERKLE_SUCCESS on success, error code on failure.
 */
merkle_error_t generate_proof_from_index(merkle_tree_t *const tree, size_t leaf_index, merkle_proof_t **proof);

/**
 * @brief Generates a Merkle proof for a leaf found using a custom finder function.
 * @param tree Pointer to the Merkle tree (must not be NULL).
 * @param finder Function pointer to locate the target leaf value.
 * @param path_length Pointer to store the length of the proof path.
 * @return Pointer to the generated proof on success, NULL on failure.
 */
 merkle_error_t generate_proof_by_finder(merkle_tree_t *const tree, value_finder finder, size_t *path_length, merkle_proof_t** proof);

/**
 * @brief Recursively deallocates a Merkle tree node and its children.
 * @param e Pointer to the Merkle node to deallocate.
 */
static void dealloc_hash_node(void *e);

/**
 * @brief Cleans up next level array and deallocates all nodes.
 * @param next_level Pointer to pointer to the next level array.
 * @param num_of_nodes Number of nodes in the array.
 */
static void clean_up_next_level(merkle_node_t ***next_level, size_t num_of_nodes);

/**
 * @brief Cleans up tree structure and deallocates memory.
 * @param tree_ptr Pointer to pointer to the tree to clean up.
 */
static void clean_up_tree(merkle_tree_t **tree_ptr);

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
static merkle_error_t build_tree_from_queue(queue_t *queue, merkle_tree_t *tree);

/**
 * @brief Adds proof path information for a node and its siblings.
 * @param parent Parent node containing siblings.
 * @param node The current node in the proof path.
 * @param proof_item Proof item to populate with sibling information.
 * @return MERKLE_SUCCESS on success, error code otherwise.
 */
static merkle_error_t add_proof_path(merkle_node_t *parent, merkle_node_t *node, merkle_proof_item_t *proof_item);

static merkle_error_t generate_proof(merkle_tree_t *const tree, size_t leaf_index, merkle_proof_t **proof);

/**
 * @brief Initializes a new Merkle tree structure.
 * @param tree Pointer to tree pointer to initialize.
 * @param leafs Number of leaf nodes the tree will contain.
 * @return MERKLE_SUCCESS on success, error code otherwise.
 */
static merkle_error_t init_tree(merkle_tree_t **tree, size_t leafs, size_t branching_factor){
  // Validate input parameters
  if(!tree){
    return MERKLE_NULL_ARG;
  }

  // Initialize output pointer
  *tree = NULL;
  merkle_tree_t *tr;
  ALLOC_AND_INIT_SIMPLE(tr, 1);
  
  if(!tr){
    return MERKLE_FAILED_MEM_ALLOC;
  }

  ALLOC_AND_INIT_SIMPLE(tr->leaves, leafs);

  if(tr->leaves == NULL){
      MFree(tr);
      return MERKLE_FAILED_MEM_ALLOC;
  }

  // Set leaf count and return initialized tree
  tr->leaf_count = leafs;
  tr->branching_factor = branching_factor;
  RW_LOCK_INIT(&tr->lock);
  *tree = tr;
  return MERKLE_SUCCESS;
}

static void dealloc_hash_node(void *e) {
  // Check for null pointer
  if (!e) {
    return;
  }

  merkle_node_t *node = e;

  // Recursively deallocate all children
  if(node->children){
    for(size_t i = 0; i < node->child_count; ++i){
        dealloc_hash_node(node->children[i]);
    }

    MFree(node->children);
  }

  // Free data if it exists (only for leaf nodes)
  if(node->data) {
      MFree(node->data);
  }
  
  // Free the node itself
  MFree(node);
}

static void clean_up_next_level(merkle_node_t ***next_level, size_t num_of_nodes) {
  // Check for null pointers
  if (!next_level || !(*next_level)) {
    return;
  }

  // Deallocate all nodes in the array
  for (size_t nd = 0; nd < num_of_nodes; ++nd) {
    if ((*next_level)[nd]) {
      dealloc_hash_node((*next_level)[nd]);
    }
  }

  // Free the array and null the pointer
  MFree(*next_level);
  *next_level = NULL;
}

static void clean_up_tree(merkle_tree_t **tree_ptr) {
  // Check for null pointers
  if (!tree_ptr || !(*tree_ptr)) {
    return;
  }

  RW_DESTROY_LOCK(&(*tree_ptr)->lock);
  // Free the leaves array and tree structure
  MFree((*tree_ptr)->leaves);
  MFree(*tree_ptr);
  *tree_ptr = NULL;
}

static merkle_error_t hash_data_block(const void *data, size_t size,
                                      unsigned char out[HASH_SIZE]) {
  // Validate input parameters
  if (!data || !out) {
    return MERKLE_NULL_ARG;
  }

  if (size == 0) {
    return MERKLE_BAD_LEN;
  }

  // Compute SHA-256 hash of the data
  SHA256((const unsigned char *)data, size, out);
  return MERKLE_SUCCESS; 
}


static merkle_error_t hash_merkle_node(merkle_node_t *parent){
  // Validate input parameter
  if (!parent) {
    return MERKLE_NULL_ARG;
  }

  // Handle case where node has no children
  if(parent->child_count == 0 || !parent->children) {
    return MERKLE_SUCCESS; // No children to hash
  }

  // Initialize SHA-256 context for combining child hashes
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  
  // Update hash with each child's hash
  for(size_t i = 0; i < parent->child_count; ++i){
    if(!parent->children[i]){
      return MERKLE_NULL_ARG;
    }

    SHA256_Update(&ctx, parent->children[i]->hash,HASH_SIZE);      
  }
   
  // Finalize the hash computation
  SHA256_Final(parent->hash,&ctx);
  return MERKLE_SUCCESS;
}

static merkle_error_t build_tree_from_queue(queue_t *queue, merkle_tree_t *tree) {
  // Local variables for managing the next level construction
  size_t next_level_alloc_count = 0;
  merkle_node_t **next_level = NULL;

  // Validate input parameters
  if (!queue || !tree) {
    return MERKLE_NULL_ARG;
  }


  if(tree->branching_factor == 0){
    return MERKLE_FAILED_TREE_BUILD;
  }

  size_t branching_factor = tree->branching_factor;

  /*
   * Process the queue level by level until a single node remains.
   * Each iteration collapses one tree level by combining up to
   * @p branching_factor child nodes under a newly allocated parent.
   */
  for (size_t tree_lvl = 0; get_queue_size(queue); tree_lvl++) {

    merkle_node_t *merkle_node = front_queue(queue);
    size_t queue_size = get_queue_size(queue);

    if(!merkle_node){
      return MERKLE_FAILED_TREE_BUILD;
    }
      
    if (IS_LAST_ELEMENT(queue_size)) {

      if(pop_queue(queue) != merkle_node){
        clean_up_next_level(&next_level, next_level_alloc_count);
        return MERKLE_FAILED_TREE_BUILD;
      }

      tree->root = merkle_node;
      clean_up_next_level(&next_level, next_level_alloc_count);
      return MERKLE_SUCCESS;
    }
      
    /* Number of parent nodes to create for this level. Each parent will
     * combine up to @p branching_factor children. */
    size_t full_nodes = (queue_size + branching_factor - 1)/branching_factor;
    tree->levels++;

    if(IS_LEAF_NODES_LEVEL(tree_lvl)) {
      ALLOC_AND_INIT_SIMPLE(next_level, full_nodes);

      if(!next_level){
        return MERKLE_FAILED_MEM_ALLOC;
      }

      next_level_alloc_count = full_nodes;
    }

    // clear all the pointers in next_level to be prepared to process the queue
    memset(next_level, 0, full_nodes * sizeof(merkle_node_t *));


    /* Build the next level of the tree by grouping children under new parents */
    for(size_t i = 0; i < full_nodes; ++i) {
      merkle_node_t *parent_node;
      ALLOC_AND_INIT_SIMPLE(parent_node, 1);

      if(!parent_node){
        clean_up_next_level(&next_level, next_level_alloc_count);
        return MERKLE_FAILED_MEM_ALLOC;
      }

      next_level[i] = parent_node;

      /* Request up to branching_factor children from the queue.  The actual
       * number dequeued is returned in @p dequed. */
      size_t dequed = branching_factor;

      queue_result_t combo_result = deque_n(
        queue,
        &dequed,
        (void ***)&(parent_node->children));

      if(combo_result != QUEUE_OK){
        clean_up_next_level(&next_level, next_level_alloc_count);
        return MERKLE_FAILED_TREE_BUILD;
      }

      parent_node->child_count = dequed;

      /* Compute the parent's hash from its children. */
      merkle_error_t ret_code = hash_merkle_node(parent_node);

      for(size_t child_idx = 0; child_idx < dequed; ++child_idx) {
        parent_node->children[child_idx]->index_in_parent = child_idx;
        parent_node->children[child_idx]->parent = parent_node;
      }

      if (ret_code != MERKLE_SUCCESS) {
        clean_up_next_level(&next_level, next_level_alloc_count);
        return ret_code;
      }
    }

    for(size_t i = 0; i < next_level_alloc_count; ++i){

      /* Enqueue the newly created parent for processing in the next level. */
      if(next_level[i] && push_queue(queue, next_level[i]) != QUEUE_OK){
        clean_up_next_level(&next_level, next_level_alloc_count);
        return MERKLE_FAILED_TREE_BUILD;
      }

      //next_level[i] should be NULLified since the ownership of the pointer has moved to the queue
      next_level[i] = NULL;
    }
  }

  clean_up_next_level(&next_level, next_level_alloc_count);
  return MERKLE_SUCCESS;
}

void dealloc_merkle_tree(merkle_tree_t *tree) {
  // Check for null pointer
  if (!tree){
    return;
  }

  // Recursively deallocate the entire tree starting from root
  dealloc_hash_node(tree->root);
  // Clean up the tree structure itself
  clean_up_tree(&tree);
}

merkle_tree_t *create_merkle_tree(const void **data, const size_t *size,
                                  size_t count, size_t branching_factor) {
  merkle_tree_t *tree = NULL;
  queue_t *queue = NULL;

  // Validate input parameters
  if (!(data && size && count > 0 && branching_factor > 0)) {
    return NULL;
  }

  // Initialize signal protection to catch segfaults gracefully
  merkle_init_signal_protection();

  // Initialize the tree structure
  if(init_tree(&tree,count,branching_factor) != MERKLE_SUCCESS){
    merkle_cleanup_signal_protection();
    return NULL;
  }

  TRY{

    // Initialize variables for tree construction
    size_t leaf_idx = 0;
    queue = init_queue();

    if(!queue){
      THROW;
    }

    bool success = 1;
    // Process each data block to create leaf nodes
    for (size_t i = 0; i < count && success; ++i) {
      
      // Safe access pattern - catch segfaults if count > actual array size
      SAFE_ACCESS_TRY {
        // Validate current data block
        if (!data[i] || !size[i]) {
          success = 0;
          break;
        }
      } SAFE_ACCESS_CATCH {
        // Segfault occurred - count parameter is incorrect
        success = 0;
        break;
      } SAFE_ACCESS_END;
      
      if (!success) break;
      
      // Allocate memory for new leaf node
      merkle_node_t *merkle_node;
      ALLOC_AND_INIT_SIMPLE(merkle_node, 1);

      if(!merkle_node){
        success = false;
        break;
      }

      merkle_node->data = MMalloc(size[i]);

      if(merkle_node->data == NULL){
        dealloc_hash_node(merkle_node);
        success = false;
        break;
      }

      // Copy the data into the node and compute hash - with protection against invalid data
      SAFE_ACCESS_TRY {
        memcpy(merkle_node->data,data[i],size[i]);
        
        // Compute the hash of the data block
        if(hash_data_block(data[i], size[i],merkle_node->hash) != MERKLE_SUCCESS){
          dealloc_hash_node(merkle_node);
          success = false;
          break;
        }
      } SAFE_ACCESS_CATCH {
        // Segfault occurred during data access
        dealloc_hash_node(merkle_node);
        success = false;
        break;
      } SAFE_ACCESS_END;
      
      if (!success) break;

      // Add the leaf node to the processing queue
      if(push_queue(queue,merkle_node) != QUEUE_OK){
        dealloc_hash_node(merkle_node);
        success = false;
        break;
      }
      // Store reference to the leaf node in the tree
      (tree->leaves)[leaf_idx++] = merkle_node;
    }

    if(!success){
      THROW;
    }
    
    RW_WRITE_LOCK(&tree->lock);
    // Build the internal tree structure from the leaf nodes
    if (build_tree_from_queue(queue, tree) != MERKLE_SUCCESS) {
      RW_WRITE_UNLOCK(&tree->lock);
      THROW;
    }

    RW_WRITE_UNLOCK(&tree->lock);

    // Clean up the temporary queue and return the completed tree
    free_queue(queue, dealloc_hash_node);
    merkle_cleanup_signal_protection();
    return tree;

  } CATCH();
  
  free_queue(queue,dealloc_hash_node);
  clean_up_tree(&tree);
  merkle_cleanup_signal_protection();
  return NULL;
}

merkle_error_t get_tree_hash(merkle_tree_t * const tree, unsigned char copy_into[HASH_SIZE]) {
  // Validate input parameters
  if (!tree || !copy_into) {
    return MERKLE_NULL_ARG;
  }

  RW_READ_LOCK(&tree->lock);

  // Check if tree is properly constructed
  if (!tree->root || tree->leaf_count == 0) {
    RW_READ_UNLOCK(&tree->lock);
    return MERKLE_BAD_ARG;
  }

  // Copy the root hash to the output buffer
  memcpy(copy_into, tree->root->hash, HASH_SIZE);
  RW_READ_UNLOCK(&tree->lock);
  return MERKLE_SUCCESS;
}

merkle_error_t generate_proof_by_finder(merkle_tree_t *const tree, value_finder finder, size_t *path_length, merkle_proof_t** proof){
  
  // Validate input parameters
  if(!tree || !finder || !proof){
      return MERKLE_BAD_ARG;
  }

  merkle_error_t result = MERKLE_SUCCESS;
  RW_READ_LOCK(&tree->lock);

  for(size_t i = 0; i < tree->leaf_count && result == MERKLE_SUCCESS; ++i){
    if(finder(tree->leaves[i]->data)){
      result = generate_proof(tree,i,proof);
      break;
    }
  }

  RW_READ_UNLOCK(&tree->lock);
  return result;
}

merkle_error_t generate_proof_from_index(merkle_tree_t *const tree, size_t leaf_index, merkle_proof_t **proof){
  if(!tree || !proof){
    return MERKLE_BAD_ARG;
  }
  
  RW_READ_LOCK(&tree->lock);
  merkle_error_t result = generate_proof(tree,leaf_index,proof);
  RW_READ_UNLOCK(&tree->lock);
  return result;
}

static merkle_error_t generate_proof(merkle_tree_t *const tree, size_t leaf_index, merkle_proof_t **proof){
  
  // Validate input parameters
  if(!tree || tree->leaf_count <= leaf_index || !proof){
      return MERKLE_BAD_ARG;
  }

  // Initialize local variables
  merkle_error_t ret = MERKLE_SUCCESS;
  *proof = NULL;
  merkle_proof_t *result = NULL;

  TRY{

    ALLOC_AND_INIT_SIMPLE(result,1);

    if(!result) {
      ret = MERKLE_FAILED_MEM_ALLOC;
      THROW;
    }

    result->path_length = tree->levels;
    result->leaf_index = leaf_index;
    result->branching_factor = tree->branching_factor;

    ALLOC_AND_INIT_SIMPLE(result->path,tree->levels);

    if(!result->path){
      ret = MERKLE_FAILED_MEM_ALLOC;
      THROW;
    }

    merkle_node_t *node = tree->leaves[leaf_index];
    bool success = 1;

    for(size_t path_len = 0; path_len < result->path_length && node->parent && success; ++path_len){
        merkle_node_t *prt = node->parent;
        ALLOC_AND_INIT(merkle_proof_item_t,proof_item,1);

        result->path[path_len] = proof_item;

        if(!proof_item){
          success = 0;
          break;
        }

        ALLOC_AND_INIT_SIMPLE(proof_item->sibling_hashes, prt->child_count - 1);
        proof_item->sibling_count = prt->child_count - 1;

        ret = add_proof_path(node->parent,node,proof_item);
        success = ret == MERKLE_SUCCESS;
        node = node->parent;
    }

    if(!success){
      THROW;
    }

    *proof = result;
    return ret;

  }CATCH();

  for(size_t idx = 0; idx != tree->levels; ++idx){
    if(!result->path[idx]) {
      continue;
    }

    MFree(result->path[idx]->sibling_hashes);
    MFree(result->path[idx]);
  }

  MFree(result->path);
  MFree(result);
  return ret;
}

static merkle_error_t add_proof_path(merkle_node_t *parent, merkle_node_t *node, merkle_proof_item_t *proof_item){
    merkle_error_t result = MERKLE_SUCCESS;
    
    TRY{
        if(!parent || !node || !proof_item){
          result = MERKLE_NULL_ARG;
          THROW;      
        }
        
        for(size_t i = 0,j = 0; i < parent->child_count; ++i){
          if(parent->children[i] != node){
            memcpy(&(proof_item->sibling_hashes[0][j]),parent->children[i]->hash,HASH_SIZE);
            j++;
          }else{
            proof_item->node_position = i;
          }
        }

    }CATCH();

    return result;
}

