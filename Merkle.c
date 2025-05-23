#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Merkle.h"
#include "Utils.h"

#define HASH_SIZE (32)
#define DATA_TYPE (1)
#define HASH_TYPE (2)
#define DELIMITER_TYPE (3)

typedef struct queue queue;

typedef enum deallocation_policy {
  Dont_Dealloc = 0,
  Dealloc = 1
} deallocation_policy;

static deallocator get_queue_dealloc(deallocation_policy policy) {
  if (policy == Dealloc) {
    return &free;
  }

  return NULL;
}

/* auxiliry methods */
static void *Merkle_Malloc(size_t size, const char *file, int line) {

#ifdef MERKLE_DEBUG
  printf("allocating %zu bytes in file %s line %d", size, file, line);
#endif

  void *res = calloc(1, size);

#ifdef MERKLE_DEBUG
  if (!res) {
    printf(file, "failed allocating %zu in file %s line %d", size, file, line);
  }
#endif

  return res;
}

static void Merkle_Free(void *d, const char *file, int line) {
#ifdef MERKLE_DEBUG
  printf("freeing in file %s line %d", file, line);
#endif
  free(d);
}

/* data types */
typedef struct {
  unsigned char hash[HASH_SIZE];
  struct MerkleNode *left, *right;
} MerkleNode;

// used for merkle proof
typedef struct {
  unsigned char hash[HASH_SIZE];
  unsigned char is_left; // 1 = sibling is on the left, 0 = on the right
} MerkleProofItem;

struct MerkleTree {
  MerkleNode *root;
  MerkleNode **leaves;
  size_t leaf_count;
};

typedef struct queue_element {
  union {
    struct {
      void *d;
      size_t size;
    } data;

    MerkleNode *mnode;
  };

  int queue_element_type;
} queue_element;

// Private Methods

static void dealloc_queue_element(void *e) {
  queue_element *qe = e;

  if (!qe || qe->queue_element_type == DELIMITER_TYPE) {
    return;
  }

  MFree(qe);
}

static void build_tree_from_queue(queue *queue, struct MerkleTree **result) {
  if (unlikely((!queue || !result))) {
    return;
  }

  *result = MMalloc(sizeof *result);

  while (get_queue_size(queue)) {
    queue_element *element = front_queue(queue);

    pop_queue(queue, get_queue_dealloc(Dont_Dealloc));

    if (element->queue_element_type == DATA_TYPE) {
      queue_element *h_el = MMalloc(sizeof *h_el);

      if (unlikely(!h_el)) {
        MFree(element);
        goto failure;
      }

      h_el->queue_element_type = HASH_TYPE;

      if (unlikely(hash_data_block(element->data.d, element->data.size,
                                   &(h_el->mnode)) != Success)) {
        MFree(h_el);
        MFree(element);
        goto failure;
      }

      queue_push(queue, h_el);
    } else {
      //{queue size > 0 && queue only has HASH_TYPES}

      // extrace another element from the q since we will need to hash them
      // togther if no such element exists then this is the root of the tree

      if (unlikely(!get_queue_size(queue))) {
        (*result)->root = element->mnode;
      }
    }
  }

  goto success;

failure:

success:
}

static M_Error hash_data_block(const void *data, size_t size,
                               unsigned char out[HASH_SIZE]) {
  if (unlikely((!data || !out))) {
    return E_NULL_ARG;
  }

  if (unlikely(size == 0)) {
    return E_BAD_LEN;
  }

  SHA256((const unsigned char *)data, size, out);
  return Success;
}

struct MerkleTree *merkle_tree_create(const void **data, const size_t *size,
                                      unsigned int count) {
  struct MerkleTree *res = NULL;
  queue_element delimiter = {.queue_element_type = DELIMITER_TYPE};

  if (unlikely(!(data && *data && size && *size > 0 && count > 0))) {
    return NULL;
  }

  queue *queue = init_queue();

  for (int i = 0; i < count; ++i) {
    if (!data[i] || !size[i]) {
      goto cleanup;
    }

    queue_element *element = MMalloc(sizeof *element);

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
    queue_element *element = MMalloc(sizeof *element);

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
