/**
 * @file merkle_queue.c
 * @brief Implementation of queue data structure for Merkle tree construction.
 *
 * This file contains the implementation of a queue_t data structure
 * specifically designed for use in the Merkle tree construction process. It
 * includes functions for initialization, memory management, and basic queue_t
 * operations. The implementation uses a sentinel node design for efficient
 * queue operations.
 *
 * @author Guy Alster
 * @date May 24, 2025
 */

#include <stdlib.h>
#include "MerkleQueue.h"
#include "Utils.h"

/**
 * @brief Returns the minimum of two values.
 * @param a First value.
 * @param b Second value.
 * @return The smaller of the two values.
 */
#define min(a, b) ((a) < (b) ? (a) : (b))

/**
 * @struct queue_node
 * @brief Internal structure representing a node in the queue.
 */
typedef struct queue_node {
  void *value;              /**< Pointer to the stored data. */
  struct queue_node *next;  /**< Pointer to the next node in the queue. */
} queue_node;

/**
 * @struct queue
 * @brief Internal structure representing the queue.
 * 
 * Uses a sentinel node design where head points to a dummy node,
 * and the actual queue elements start from head->next.
 */
struct queue {
  queue_node *head;    /**< Pointer to sentinel node. */
  queue_node *tail;    /**< Pointer to the last actual node (or sentinel if empty). */
  size_t queue_size;   /**< Current number of elements in the queue. */
};

/**
 * @brief Initializes a new queue with a sentinel node and returns a pointer to it.
 * 
 * Creates a new queue using the sentinel node pattern for efficient operations.
 * The sentinel node simplifies insertion and deletion logic by eliminating
 * special cases for empty queues.
 * 
 * @return Pointer to the newly created queue, or NULL if memory allocation fails.
 * 
 * @note The queue starts empty with head and tail both pointing to the sentinel node.
 */
queue_t *init_queue(void) {
  queue_node *sent_node = MMalloc(sizeof *sent_node);
  queue_t *q = NULL;

  if (!sent_node) {
    return NULL;
  }

  q = MMalloc(sizeof *q);

  if (!q) {
    MFree(sent_node); // Fix memory leak
    return NULL;
  }

  q->head = q->tail = sent_node;
  q->queue_size = 0;
  sent_node->next = NULL;
  sent_node->value = NULL;
  
  return q;
}

/**
 * @brief Frees all memory associated with the queue, including its nodes and their values.
 * 
 * Iterates through all nodes in the queue (excluding the sentinel) and frees them.
 * If a deallocator function is provided, it's called for each element's data.
 * Finally frees the sentinel node and queue structure.
 * 
 * @param q Pointer to the queue to free. Safe to pass NULL.
 * @param dealloc Optional deallocator function for element data. Can be NULL.
 */
void free_queue(queue_t *q, deallocator dealloc) {

  if (!q) {
    return;
  }

  queue_node *head = q->head;
  queue_node *d = head->next;  // Skip sentinel node

  // Free all data nodes
  while (d) {
    queue_node *next = d->next;

    if (dealloc && d->value) {
      dealloc(d->value);
    }

    MFree(d);
    d = next;
  }

  // Free sentinel node and queue structure
  MFree(head);
  MFree(q);
}

/**
 * @brief Adds a new element to the end of the queue.
 * 
 * Creates a new node and links it after the current tail.
 * Updates the tail pointer and increments the queue size.
 * 
 * @param q Pointer to the queue.
 * @param data Pointer to the data to store. Can be NULL.
 * @return QUEUE_OK on success, error code on failure.
 */
queue_result_t push_queue(queue_t *q, void *data) {

  if (!q) {
    return QUEUE_NULL_PTR;
  }

  queue_node *ins = MMalloc(sizeof *ins);

  if (!ins) {
    return QUEUE_OUT_OF_MEMORY;
  }

  ins->next = NULL;
  ins->value = data;

  // Link the new node after current tail
  q->tail->next = ins;
  q->tail = ins;
  q->queue_size++;
  
  return QUEUE_OK;
}

/**
 * @brief Removes the front element from the queue and returns its value.
 * 
 * Removes the first actual data node (head->next) and returns its value.
 * Updates head->next to point to the following node. If this was the last
 * element, resets tail to point back to the sentinel node.
 * 
 * @param q Pointer to the queue.
 * @return Pointer to the data of the removed element, or NULL if queue is empty/NULL.
 */
void *pop_queue(queue_t *q) {
  if (!q || q->tail == q->head) {  // Empty queue check
    return NULL;
  }

  void *result = q->head->next->value;
  queue_node *to_free = q->head->next;

  // Update head to skip the removed node
  q->head->next = to_free->next;
  
  // If we removed the last element, reset tail to sentinel
  if (!to_free->next) {
    q->tail = q->head;
  }

  MFree(to_free); // Only free the node, not the data
  q->queue_size--;
  
  return result;
}

/**
 * @brief Returns the value of the front element in the queue without removing it.
 * 
 * Peeks at the first actual data element (head->next) without modifying
 * the queue structure.
 * 
 * @param q Pointer to the queue.
 * @return Pointer to the front element's data, or NULL if queue is empty/NULL.
 */
void *front_queue(queue_t *q) {

  if (!q) {
    return NULL;
  }

  if (!q->head->next) {  // Check if queue is empty
    return NULL;
  }

  return q->head->next->value;
}

/**
 * @brief Returns the value of the last element in the queue without removing it.
 * 
 * Returns the data stored in the tail node. If the queue is empty,
 * tail points to the sentinel node and we return NULL.
 * 
 * @param q Pointer to the queue.
 * @return Pointer to the rear element's data, or NULL if queue is empty/NULL.
 */
void *back_queue(queue_t *q) {

  if (!q) {
    return NULL;
  }

  if (q->tail == q->head) {  // Empty queue check
    return NULL;
  }

  return q->tail->value;
}

/**
 * @brief Returns the current size of the queue.
 * 
 * Returns the cached size value that's maintained during push/pop operations.
 * This provides O(1) size queries.
 * 
 * @param q Pointer to the queue.
 * @return Number of elements in the queue, or 0 if queue is NULL.
 */
size_t get_queue_size(queue_t *q) {

  if (!q) {
    return 0;
  }

  return q->queue_size;
}

/**
 * @brief Removes up to n elements from the front of the queue.
 * 
 * Dequeues the minimum of the requested count and available elements.
 * Allocates an array to hold the pointers and populates it by calling
 * pop_queue() repeatedly. Updates the count parameter with the actual
 * number of elements returned.
 * 
 * @param q Pointer to the queue.
 * @param count Pointer to requested count (input) and actual count (output).
 * @param result Pointer to store the allocated array of element pointers.
 * @return QUEUE_OK on success, error code on failure.
 * 
 * @note Caller is responsible for freeing the returned array and managing
 *       the element data according to their ownership model.
 * @note If memory allocation fails, *count is set to 0.
 */
queue_result_t deque_n(queue_t *q, size_t *count, void ***result){
  if(!count){
    return QUEUE_NULL_PTR;
  }

  size_t size = get_queue_size(q);
  size_t deque = min((*count), size);

  // Allocate array to hold the dequeued element pointers
  *result = MMalloc(deque * sizeof(void *));
  
  if(!*result){
    *count = 0;
    return QUEUE_OUT_OF_MEMORY;
  }

  // Dequeue elements and store in result array
  for(size_t i = 0; i < deque; ++i){
    (*result)[i] = pop_queue(q); 
  }

  // Update count with actual number dequeued
  (*count) = deque;
  return QUEUE_OK;
}
