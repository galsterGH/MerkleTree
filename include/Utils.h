/*
 * Utils.h
 *
 * This header file provides utility functions and definitions for various data
 * structures and operations. It is designed to be a general-purpose utility
 * library that can be extended to include additional functionality as needed.
 *
 * Author: Guy Alster
 * Date: May 22, 2025
 */

#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/**
 * @def likely(x)
 * @brief Hint to the compiler that a condition is likely true.
 *
 * This macro is used for branch prediction optimization. It expands to
 * a compiler built-in that informs the optimizer that the expression @p x
 * is expected to evaluate to a non-zero value most of the time.
 */
#define likely(x) __builtin_expect(!!(x), 1)

/**
 * @def unlikely(x)
 * @brief Hint to the compiler that a condition is unlikely true.
 *
 * Similar to ::likely(), this macro marks the expression @p x as usually
 * false, allowing the compiler to optimize the branch layout accordingly.
 */
#define unlikely(x) __builtin_expect(!!(x), 0)

/**
 * @def MMalloc(x)
 * @brief Convenience wrapper around Merkle_Malloc with file/line info.
 */
#define MMalloc(x) Merkle_Malloc(x, __FILE__, __LINE__)

/**
 * @def MFree(x)
 * @brief Convenience wrapper around Merkle_Free with file/line info.
 */
#define MFree(x) Merkle_Free(x, __FILE__, __LINE__)

/**
 * @brief Allocate zero-initialized memory with debug location information.
 *
 * @param size Number of bytes to allocate.
 * @param file Source file requesting the allocation.
 * @param line Line number in @p file where the allocation occurs.
 * @return Pointer to allocated memory or NULL on failure.
 */
void *Merkle_Malloc(size_t size, const char *file, int line);

/**
 * @brief Free memory allocated by Merkle_Malloc().
 *
 * @param d Pointer to memory to free.
 * @param file Source file requesting the deallocation.
 * @param line Line number in @p file where the free occurs.
 */
void Merkle_Free(void *d, const char *file, int line);

#endif
