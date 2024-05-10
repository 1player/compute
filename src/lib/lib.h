#ifndef LIB_H
#define LIB_H

#include "array.h"

typedef struct FIFOEntry FIFOEntry;

typedef struct FIFO {
  pthread_mutex_t lock;
  FIFOEntry *front;
  FIFOEntry *back;
} FIFO;

void fifo_init(FIFO *mailbox);
void fifo_push(FIFO *mailbox, void *data);
void *fifo_pop(FIFO *mailbox);

#endif
