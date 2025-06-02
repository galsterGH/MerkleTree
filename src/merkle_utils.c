/**
 * @file merkle_utils.c
 * @brief Utility functions for the Merkle tree implementation.
 *
 * This file contains utility functions and definitions for the Merkle tree
 * implementation, including memory management functions with debugging support.
 *
 * @author Guy Alster
 * @date May 24, 2025
 */

#include <stdlib.h>
#include "Utils.h"


/**
 * @brief Allocates zero-initialized memory with optional debug information.
 *
 * This function allocates a memory block of the given size, zero-initializes
 * it, and, if MERKLE_DEBUG is defined, prints debug information about the
 * allocation.
 *
 * @param size Number of bytes to allocate.
 * @param file Source file name where the allocation is requested.
 * @param line Line number in the source file where the allocation is requested.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
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

/**
 * @brief Frees memory previously allocated by Merkle_Malloc, with optional
 * debug information.
 *
 * This function frees the memory pointed to by @p d. If MERKLE_DEBUG is
 * defined, it prints debug information about the deallocation.
 *
 * @param d Pointer to the memory to free.
 * @param file Source file name where the deallocation is requested.
 * @param line Line number in the source file where the deallocation is
 * requested.
 */
void Merkle_Free(void *d, const char *file, int line) {
#ifdef MERKLE_DEBUG
  printf("freeing in file %s line %d", file, line);
#endif
  free(d);
}