/*
 * Utils.c
 *
 * This file contains the implementation of a queue data structure, including
 * functions for initialization, memory management, and basic queue operations
 * such as push, pop, and access to the front and back elements.
 *
 * Author: [Your Name]
 * Date: May 22, 2025
 */

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
} queue;

// { true }
// Initializes a new queue with a sentinel node and returns a pointer to it.
queue *init_queue() {
  queue_node *sent_node = calloc(1, sizeof *sent_node);
  queue *q = NULL;

  if (unlikely(!sent_node)) {
    return NULL;
  }

  q = calloc(1, sizeof *q);

  if (unlikely(!q)) {
    return NULL;
  }

  q->head = q->tail = sent_node;
  return q;
  // { q != NULL && q->head == q->tail && q->queue_size == 0 }
}

// { q != NULL }
// Frees all memory associated with the queue, including its nodes and their
// values.
void free_queue(queue *q, deallocator dealloc) {

  if (unlikely(!q)) {
    return;
  }

  queue_node *head = q->head;
  queue_node *d = head->next;

  while (d) {
    queue_node *next = d->next;

    if (dealloc) {
      dealloc(d->value);
    }

    free(d);
    d = next;
  }
  free(head);
  free(q);
  // { q and all its nodes are deallocated }
}

// { q != NULL }
// Adds a new element to the end of the queue.
void push_queue(queue *q, void *data) {

  if (unlikely(!q)) {
    return;
  }

  queue_node *ins = calloc(1, sizeof *ins);

  if (unlikely(!ins)) {
    return;
  }

  ins->value = data;

  //{q->tail == q->head || q->tail != q->head}
  q->tail->next = ins;
  q->tail = ins;
  q->queue_size++;
  // { q->queue_size is incremented by 1 and data is added to the tail }
}

// { q != NULL && q->queue_size > 0 }
// Removes the front element from the queue and deallocates its memory.
void pop_queue(queue *q, deallocator dealloc) {
  queue_node *tail, *temp;

  if (unlikely(!q)) {
    return;
  }

  if (q->tail == q->head) {
    return;
  }

  // invariant: q has at least one elements after the sentinal
  temp = q->head->next->next;

  if (dealloc) {
    dealloc(q->head->next->value);
  }

  free(q->head->next);
  q->head->next = temp;

  if (!temp) {
    q->tail = q->head;
  }

  q->queue_size--;
  // { q->queue_size is decremented by 1 and the front element is removed }
}

// { q != NULL }
// Returns the value of the front element in the queue without removing it.
void *front_queue(queue *q) {

  if (unlikely(!q)) {
    return NULL;
  }

  return q->head->next->value;
  // { returns the value of the front element or NULL if the queue is empty }
}

// { q != NULL }
// Returns the value of the last element in the queue without removing it.
void *back_queue(queue *q) {

  if (unlikely(!q)) {
    return NULL;
  }

  return q->tail->value;
  // { returns the value of the last element or NULL if the queue is empty }
}

// { q != NULL }
// Returns the current size of the queue.
size_t get_queue_size(queue *q) {

  if (unlikely(!q)) {
    return 0;
  }

  return q->queue_size;
  // { returns the current size of the queue }
}