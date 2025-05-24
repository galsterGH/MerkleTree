/*
 * MerkleQueue.c
 *
 * This file contains the implementation of a queue_t data structure
 * specifically designed for use in the Merkle tree construction process. It
 * includes functions for initialization, memory management, and basic queue_t
 * operations.
 *
 * Author: [Your Name]
 * Date: May 24, 2025
 */

#include "MerkleQueue.h"
#include "Utils.h"
#include <stdlib.h>

typedef struct queue_node {
  void *value;
  struct queue_node *next;
} queue_node;

typedef struct queue {
  queue_node *head;
  queue_node *tail;
  size_t queue_size;
} queue_t;

// Initializes a new queue_t with a sentinel node and returns a pointer to it.
queue_t *init_queue() {
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
  return q;
}

// Frees all memory associated with the queue, including its nodes and their
// values.
void free_queue(queue_t *q, deallocator dealloc) {

  if (!q) {
    return;
  }

  queue_node *head = q->head;
  queue_node *d = head->next;

  while (d) {
    queue_node *next = d->next;

    if (dealloc) {
      dealloc(d->value);
    }

    MFree(d);
    d = next;
  }

  MFree(head);
  MFree(q);
}

// Adds a new element to the end of the queue.
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

  q->tail->next = ins;
  q->tail = ins;
  q->queue_size++;
  return QUEUE_OK;
}

// Removes the front element from the queue and returns its value.
void *pop_queue(queue_t *q) {
  if (!q || q->tail == q->head) {
    return NULL;
  }

  void *result = q->head->next->value;
  queue_node *to_free = q->head->next;

  q->head->next = to_free->next;
  if (!to_free->next) {
    q->tail = q->head;
  }

  MFree(to_free); // Only free the node, not the data
  q->queue_size--;
  return result;
}

// Returns the value of the front element in the queue without removing it.
void *front_queue(queue_t *q) {

  if (!q) {
    return NULL;
  }

  if (!q->head->next) {
    return NULL;
  }

  return q->head->next->value;
}

// Returns the value of the last element in the queue without removing it.
void *back_queue(queue_t *q) {

  if (!q) {
    return NULL;
  }

  if (q->tail == q->head) {
    return NULL;
  }

  return q->tail->value;
}

// Returns the current size of the queue.
size_t get_queue_size(queue_t *q) {

  if (!q) {
    return 0;
  }

  return q->queue_size;
}
