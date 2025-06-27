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
#include <signal.h>
#include <setjmp.h>
#include "Utils.h"

// Thread-local storage for signal handling
__thread jmp_buf merkle_segv_buf;
__thread volatile int merkle_segv_occurred = 0;

// Store original signal handlers for restoration
static __thread void (*original_segv_handler)(int) = NULL;
static __thread void (*original_bus_handler)(int) = NULL;
static __thread void (*original_abrt_handler)(int) = NULL;

/**
 * @brief Signal handler for memory access violations
 */
static void merkle_signal_handler(int sig) {
    merkle_segv_occurred = 1;
    longjmp(merkle_segv_buf, sig);
}

/**
 * @brief Initialize signal protection for current thread
 */
void merkle_init_signal_protection(void) {
    // Install signal handlers for memory access violations
    original_segv_handler = signal(SIGSEGV, merkle_signal_handler);
    
#ifdef SIGBUS  // Not available on all platforms
    original_bus_handler = signal(SIGBUS, merkle_signal_handler);
#endif

#ifdef SIGABRT
    original_abrt_handler = signal(SIGABRT, merkle_signal_handler);
#endif

    merkle_segv_occurred = 0;
}

/**
 * @brief Cleanup signal protection for current thread
 */
void merkle_cleanup_signal_protection(void) {
    // Restore original signal handlers
    if (original_segv_handler != NULL) {
        signal(SIGSEGV, original_segv_handler);
        original_segv_handler = NULL;
    }
    
#ifdef SIGBUS
    if (original_bus_handler != NULL) {
        signal(SIGBUS, original_bus_handler);
        original_bus_handler = NULL;
    }
#endif

#ifdef SIGABRT
    if (original_abrt_handler != NULL) {
        signal(SIGABRT, original_abrt_handler);
        original_abrt_handler = NULL;
    }
#endif

    merkle_segv_occurred = 0;
}


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
