/*
 * Utils.h
 *
 * This header file provides utility functions and definitions for various data
 * structures and operations. It is designed to be a general-purpose utility
 * library that can be extended to include additional functionality as needed.
 *
 * Author: [Your Name]
 * Date: May 22, 2025
 */

#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

// Macro to indicate that a condition is likely to be true, used for branch
// prediction optimization.
#define likely(x) __builtin_expect(!!(x), 1)

// Macro to indicate that a condition is unlikely to be true, used for branch
// prediction optimization.
#define unlikely(x) __builtin_expect(!!(x), 0)

#define MMalloc(x) Merkle_Malloc(x, __FILE__, __LINE__)
#define MFree(x) Merkle_Free(x, __FILE__, __LINE__)

void *Merkle_Malloc(size_t size, const char *file, int line);
void Merkle_Free(void *d, const char *file, int line);

#endif