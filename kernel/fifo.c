#include <assert.h>
#include <stdio.h>
#include <pthread.h>

#include "lib.h"

typedef struct FIFOEntry {
  void *data;
  struct FIFOEntry *next;
} FIFOEntry;


static FIFOEntry *alloc_entry(void *data) {
  FIFOEntry *entry = calloc(1, sizeof(FIFOEntry));
  entry->data = data;
  return entry;
}

void free_entry(FIFOEntry *entry) {
  free(entry);
}

void fifo_push(FIFO *fifo, void *data) {
  assert(pthread_mutex_lock(&fifo->lock) == 0);

  if (fifo->front) {
    fifo->back->next = alloc_entry(data);
    fifo->back = fifo->back->next;
  } else {
    fifo->front = alloc_entry(data);
    fifo->back = fifo->front;
  }

  assert(pthread_mutex_unlock(&fifo->lock) == 0);
}

void *fifo_pop(FIFO *fifo) {
  void *data = NULL;

  assert(pthread_mutex_lock(&fifo->lock) == 0);

  if (!fifo->front) {
    goto end;
  }

  FIFOEntry *old_front = fifo->front;
  data = old_front->data;
  fifo->front = old_front->next;

  free_entry(old_front);

 end:
  assert(pthread_mutex_unlock(&fifo->lock) == 0);
  return data;
}

void fifo_init(FIFO *fifo) {
  if (pthread_mutex_init(&fifo->lock, NULL) != 0) {
    perror("pthread_mutex_init");
    exit(1);
  }
  fifo->front = NULL;
  fifo->back = NULL;
}
