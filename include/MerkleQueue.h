/**
 * @file MerkleQueue.h
 * @brief Queue data structure implementation for Merkle tree construction.
 *
 * This header file provides a queue data structure implementation specifically
 * designed for use in the Merkle tree construction process. It includes
 * functions for initialization, memory management, and basic queue operations.
 * The queue uses a sentinel node design for efficient operations.
 *
 * @author Guy Alster
 * @date May 24, 2025
 */

#ifndef MERKLE_QUEUE_H
#define MERKLE_QUEUE_H

#include <stddef.h>

/**
 * @enum queue_result_t
 * @brief Result codes returned by queue operations.
 */
typedef enum {
  QUEUE_OK = 0,           /**< Operation completed successfully. */
  QUEUE_NULL_PTR,         /**< Null pointer passed to function. */
  QUEUE_OUT_OF_MEMORY,    /**< Memory allocation failed. */
  QUEUE_BAD_ARGUMENT      /**< Invalid argument passed to function. */
} queue_result_t;

/**
 * @typedef deallocator
 * @brief Function pointer type for deallocating memory for queue elements.
 * @param ptr Pointer to the memory to deallocate.
 */
typedef void (*deallocator)(void *ptr);

/**
 * @struct queue
 * @brief Opaque structure representing a queue.
 * 
 * This structure is forward declared to hide implementation details.
 * Users should only interact with the queue through the provided functions.
 */
typedef struct queue queue_t;

/**
 * @brief Initializes a new queue and returns a pointer to it.
 * 
 * Creates a new queue with a sentinel node for efficient operations.
 * The queue starts empty but ready for use.
 * 
 * @return Pointer to the newly created queue, or NULL if allocation fails.
 * 
 * @note The returned queue must be freed using free_queue() when no longer needed.
 * 
 * @see free_queue()
 */
queue_t *init_queue(void);

/**
 * @brief Frees all memory associated with the queue, including its elements.
 * 
 * Deallocates the queue structure and all nodes. If a deallocator function
 * is provided, it will be called for each element's data before freeing the node.
 * 
 * @param q Pointer to the queue to free. Can be NULL (no-op).
 * @param dealloc Function to call for deallocating element data, or NULL if
 *                elements don't need deallocation.
 * 
 * @warning After calling this function, the queue pointer becomes invalid.
 * 
 * @see init_queue(), deallocator
 */
void free_queue(queue_t *q, deallocator dealloc);

/**
 * @brief Adds a new element to the end of the queue.
 * 
 * Pushes a new element to the tail of the queue. The queue takes ownership
 * of the pointer but not the data it points to (shallow copy).
 * 
 * @param q Pointer to the queue.
 * @param data Pointer to the data to add. Can be NULL.
 * @return QUEUE_OK on success, error code otherwise.
 * @retval QUEUE_OK Operation successful.
 * @retval QUEUE_NULL_PTR Queue pointer is NULL.
 * @retval QUEUE_OUT_OF_MEMORY Memory allocation failed.
 * 
 * @note The queue stores the pointer value, not a copy of the data.
 * 
 * @see pop_queue(), front_queue()
 */
queue_result_t push_queue(queue_t *q, void *data);

/**
 * @brief Removes the front element from the queue and returns its value.
 * 
 * Removes and returns the element at the front of the queue. The caller
 * becomes responsible for any memory management of the returned data.
 * 
 * @param q Pointer to the queue.
 * @return Pointer to the data of the removed element, or NULL if queue is empty or NULL.
 * 
 * @warning The caller is responsible for freeing the returned data if needed.
 * @note This function only frees the queue node, not the element data.
 * 
 * @see push_queue(), front_queue()
 */
void *pop_queue(queue_t *q);

/**
 * @brief Returns the value of the front element without removing it.
 * 
 * Peeks at the front element of the queue without modifying the queue structure.
 * 
 * @param q Pointer to the queue.
 * @return Pointer to the data of the front element, or NULL if queue is empty or NULL.
 * 
 * @note The returned pointer is valid until the front element is removed.
 * 
 * @see back_queue(), pop_queue()
 */
void *front_queue(queue_t *q);

/**
 * @brief Returns the value of the last element without removing it.
 * 
 * Peeks at the rear element of the queue without modifying the queue structure.
 * 
 * @param q Pointer to the queue.
 * @return Pointer to the data of the rear element, or NULL if queue is empty or NULL.
 * 
 * @note The returned pointer is valid until the rear element is removed.
 * 
 * @see front_queue(), push_queue()
 */
void *back_queue(queue_t *q);

/**
 * @brief Returns the current number of elements in the queue.
 * 
 * @param q Pointer to the queue.
 * @return Number of elements in the queue, or 0 if queue is NULL.
 * 
 * @note This operation is O(1) as the size is maintained internally.
 */
size_t get_queue_size(queue_t *q);

/**
 * @brief Removes up to n elements from the front of the queue.
 * 
 * Dequeues up to the requested number of elements and returns them in an array.
 * If the queue contains fewer elements than requested, returns all available elements.
 * The count parameter is updated to reflect the actual number of elements returned.
 * 
 * @param q Pointer to the queue.
 * @param count Pointer to the requested number of elements. Updated with actual count returned.
 * @param result Pointer to store the allocated array of element pointers.
 * @return QUEUE_OK on success, error code otherwise.
 * @retval QUEUE_OK Operation successful.
 * @retval QUEUE_NULL_PTR count parameter is NULL.
 * @retval QUEUE_OUT_OF_MEMORY Memory allocation for result array failed.
 * 
 * @note The caller is responsible for freeing the returned array and its elements.
 * @note If allocation fails, *count is set to 0 and *result is undefined.
 * 
 * @warning The returned array and element ownership transfers to the caller.
 * 
 * @see pop_queue(), get_queue_size()
 */
queue_result_t deque_n(queue_t *q, size_t *count, void ***result);

#endif