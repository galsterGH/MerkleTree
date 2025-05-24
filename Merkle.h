#ifndef MERKLE_H
#define MERKLE_H

#include <stddef.h>

typedef enum {
  MERKLE_SUCCESS = 0,
  MERKLE_NULL_ARG,
  MERKLE_BAD_LEN,
  MERKLE_FAILED_TREE_BUILD
} merkle_error_t;

typedef struct merkle_tree merkle_tree_t;

merkle_tree_t *merkle_tree_create(const void **data, const size_t *sizes,
                                  size_t count);
void merkle_tree_destroy(merkle_tree_t *tree);

#endif // MERKLE_H
