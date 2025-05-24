/*
 * Utils.c
 *
 * This file contains utility functions and definitions for the Merkle tree
 * implementation, including memory management functions with debugging support.
 *
 * Author: [Your Name]
 * Date: May 24, 2025
 */

#include "Utils.h"
#include <stdlib.h>

/* Memory management methods */
void *Merkle_Malloc(size_t size, const char *file, int line) {

#ifdef MERKLE_DEBUG
  printf("allocating %zu bytes in file %s line %d", size, file, line);
#endif

  void *res = calloc(1, size);

#ifdef MERKLE_DEBUG
  if (!res) {
    printf("failed allocating %zu in file %s line %d", size, file, line);
  }
#endif

  return res;
}

void Merkle_Free(void *d, const char *file, int line) {
#ifdef MERKLE_DEBUG
  printf("freeing in file %s line %d", file, line);
#endif
  free(d);
}