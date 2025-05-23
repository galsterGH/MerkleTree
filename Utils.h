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

// Typedef for a function pointer used to deallocate memory for queue elements.
typedef void (*deallocator)(void *);

// Forward declaration of the queue structure to hide implementation details.
struct queue;

// Initializes a new queue and returns a pointer to it.
struct queue *init_queue();

// Frees all memory associated with the queue, including its elements.
void free_queue(struct queue *q, deallocator dealloc);

// Adds a new element to the end of the queue.
void push_queue(struct queue *q, void *data);

// Removes the front element from the queue and deallocates its memory.
void pop_queue(struct queue *q, deallocator dealloc);

// Returns the value of the front element in the queue without removing it.
void *front_queue(struct queue *q);

// Returns the value of the last element in the queue without removing it.
void *back_queue(struct queue *q);

// Returns the current size of the queue.
size_t get_queue_size(struct queue *q);

#endif