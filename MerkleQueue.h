/*
 * MerkleQueue.h
 *
 * This header file provides a queue data structure implementation specifically
 * designed for use in the Merkle tree construction process. It includes
 * functions for initialization, memory management, and basic queue operations.
 *
 * Author: [Your Name]
 * Date: May 24, 2025
 */

#ifndef MERKLE_QUEUE_H
#define MERKLE_QUEUE_H

#include <stddef.h>

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
void *pop_queue(struct queue *q, deallocator dealloc);

// Returns the value of the front element in the queue without removing it.
void *front_queue(struct queue *q);

// Returns the value of the last element in the queue without removing it.
void *back_queue(struct queue *q);

// Returns the current size of the queue.
size_t get_queue_size(struct queue *q);

#endif
