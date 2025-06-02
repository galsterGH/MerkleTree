/*
 * MerkleQueue.h
 *
 * This header file provides a queue data structure implementation specifically
 * designed for use in the Merkle tree construction process. It includes
 * functions for initialization, memory management, and basic queue operations.
 *
 * Author: Guy Alster
 * Date: May 24, 2025
 */

#ifndef MERKLE_QUEUE_H
#define MERKLE_QUEUE_H

#include <stddef.h>

typedef enum {
  QUEUE_OK = 0,
  QUEUE_NULL_PTR,
  QUEUE_OUT_OF_MEMORY,
  QUEUE_BAD_ARGUMENT
} queue_result_t;

// Typedef for a function pointer used to deallocate memory for queue elements.
typedef void (*deallocator)(void *);

// Forward declaration of the queue structure to hide implementation details.
typedef struct queue queue_t;

// Initializes a new queue and returns a pointer to it.
queue_t *init_queue(void);

// Frees all memory associated with the queue, including its elements.
void free_queue(queue_t *q, deallocator dealloc);

// Adds a new element to the end of the queue.
queue_result_t push_queue(queue_t *q, void *data);

// Removes the front element from the queue and deallocates its memory.
void *pop_queue(queue_t *q);

// Returns the value of the front element in the queue without removing it.
void *front_queue(queue_t *q);

// Returns the value of the last element in the queue without removing it.
void *back_queue(queue_t *q);

// Returns the current size of the queue.
size_t get_queue_size(queue_t *q);

queue_result_t deque_n(queue_t *q, size_t *count, void ***result);

#endif