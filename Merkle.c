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
#define DELIMITER_TYPE (3)

typedef enum deallocation_policy {
  Dont_Dealloc = 0,
  Dealloc = 1
} deallocation_policy;

static void free_all(void *first, ...) {
  if (!first) {
    return; // Nothing to free
  }

  // Free the first pointer
  MFree(first);

  // Set up variadic argument processing
  va_list args;
  va_start(args, first);

  void *ptr;
  // Continue freeing pointers until we hit NULL (sentinel)
  while ((ptr = va_arg(args, void *)) != NULL) {
    MFree(ptr);
  }

  va_end(args);
}

static deallocator get_queue_dealloc(deallocation_policy policy) {
  if (policy == Dealloc) {
    return &free;
  }

  return NULL;
}

/* data types */
typedef struct merkle_node {
  unsigned char hash[HASH_SIZE];
  struct merkle_node *left, *right;
} merkle_node_t;

// used for merkle proof
typedef struct merkle_proof_item {
  unsigned char hash[HASH_SIZE];
  unsigned char is_left; // 1 = sibling is on the left, 0 = on the right
} merkle_proof_item_t;

typedef struct merkle_tree {
  merkle_node_t *root;
  merkle_node_t **leaves;
  size_t leaf_count;
} merkle_tree_t;

typedef struct queue_element_t {
  union {
    struct {
      void *d;
      size_t size;
    } data;

    merkle_node_t *mnode;
  };

  int queue_element_type;
} queue_element_t;

// Private Methods
static void dealloc_hash_node(merkle_node_t *e) {
  if (unlikely(!e)) {
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
    free_all(qe->data.d, qe);
    return;
  }

  dealloc_hash_node(qe->mnode);
}

static void dealloc_queue_elements(void *first, ...) {
  if (!first) {
    return; // Nothing to free
  }

  // Free the first pointer
  dealloc_queue_element(first);

  // Set up variadic argument processing
  va_list args;
  va_start(args, first);

  void *ptr;
  // Continue freeing pointers until we hit NULL (sentinel)
  while ((ptr = va_arg(args, void *)) != NULL) {
    dealloc_queue_element(ptr);
  }

  va_end(args);
}

static merkle_error_t build_tree_from_queue(queue_t *queue,
                                            merkle_tree_t **result) {
  if (unlikely((!queue || !result))) {
    return E_NULL_ARG;
  }

  merkle_error_t ret_code = Success;
  *result = MMalloc(sizeof *result);

  while (get_queue_size(queue)) {
    queue_element_t *element = front_queue(queue);

    pop_queue(queue);

    if (element->queue_element_type == DATA_TYPE) {
      queue_element_t *h_el = MMalloc(sizeof *h_el);

      if (unlikely(!h_el)) {
        dealloc_queue_elements(element);
        ret_code = E_NULL_ARG;
        goto failure;
      }

      h_el->queue_element_type = HASH_TYPE;
      h_el->mnode = MMalloc(sizeof *(h_el->mnode));

      ret_code = hash_data_block(element->data.d, element->data.size,
                                 &(h_el->mnode->hash));

      if (unlikely(ret_code != Success)) {
        dealloc_queue_elements(h_el, element);
        goto failure;
      }

      queue_push(queue, h_el);

    } else {
      //{queue size > 0 && queue only has HASH_TYPES}

      // extrace another element from the q since we will need to hash them
      // togther if no such element exists then this is the root of the tree

      if (unlikely(!get_queue_size(queue))) {
        (*result)->root = element->mnode;
      } else {

        queue_element_t *next_element = front_queue(queue);
        pop_queue(queue, get_queue_dealloc(Dont_Dealloc));

        queue_element_t *combined = MMalloc(sizeof *combined);

        if (unlikely(!combined)) {
          dealloc_queue_elements(element, next_element);
          ret_code = E_BAD_LEN;
          goto failure;
        }

        combined->queue_element_type = HASH_TYPE;
        combined->mnode = MMalloc(sizeof *(combined->mnode));

        unsigned char out_hash[HASH_SIZE];

        ret_code =
            hash_two_nodes(element->mnode, next_element->mnode, &(out_hash[0]));

        if (ret_code != Success) {
          dealloc_queue_elements(element, next_element, combined);
          goto failure;
        }

        memcpy(&(combined->mnode->hash[0]), &(out_hash[0]), HASH_SIZE);
        combined->mnode->left = element->mnode;
        combined->mnode->right = next_element->mnode;

        // clean up the queue nodes
        element->mnode = next_element->mnode = NULL;
        dealloc_queue_elements(element, next_element);
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

static merkle_error_t hash_data_block(const void *data, size_t size,
                                      unsigned char out[HASH_SIZE]) {
  if (unlikely((!data || !out))) {
    return E_NULL_ARG;
  }

  if (unlikely(size == 0)) {
    return E_BAD_LEN;
  }

  SHA256((const unsigned char *)data, size, out);
  s return Success;
}

static merkle_error_t hash_two_nodes(merkle_node_t *n1, merkle_node_t *n2,
                                     unsigned char **out) {
  if (unlikely(!n1 || !n2 || !out || *out)) {
    return E_NULL_ARG;
  }

  unsigned char combined_hash[HASH_SIZE * 2] = {0};
  memcpy(&(combined_hash[0]), &(n1->hash[0]), HASH_SIZE);
  memcpy(&(combined_hash[HASH_SIZE]), &(n2->hash[0]), HASH_SIZE);

  return hash_data_block(&(combined_hash[0]), HASH_SIZE * 2, *out);
}

struct merkle_tree_t *merkle_tree_create(const void **data, const size_t *size,
                                         unsigned int count) {
  struct merkle_tree_t *res = NULL;

  if (unlikely(!(data && *data && size && *size > 0 && count > 0))) {
    return NULL;
  }

  queue_t *queue = init_queue();

  for (int i = 0; i < count; ++i) {
    if (!data[i] || !size[i]) {
      goto cleanup;
    }

    queue_element_t *element = MMalloc(sizeof *element);

    if (unlikely(!element)) {
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

    if (unlikely(!element)) {
      goto cleanup;
    }

    element->data.d = data[count - 1];
    element->data.size = size[count - 1];
    element->queue_element_type = DATA_TYPE;
    push_queue(queue, element);
  }

  build_tree_from_queue(queue, &res);

cleanup:
  free_queue(queue, dealloc_queue_element);
  return res;
}

int main() { return 0; }
