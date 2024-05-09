#ifndef CORE_H
#define CORE_H

#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

#include "array.h"

// FIFO

typedef struct FIFOEntry FIFOEntry;

typedef struct FIFO {
  pthread_mutex_t lock;
  FIFOEntry *front;
  FIFOEntry *back;
} FIFO;

void fifo_init(FIFO *mailbox);
void fifo_push(FIFO *mailbox, void *data);
void *fifo_pop(FIFO *mailbox);

// Message

typedef struct Message {
  char *name;
} Message;

// Actor

typedef unsigned int PID;

typedef void (*HandlerFunc)(void *, Message *);

typedef struct Actor {
  PID pid;

  atomic_bool is_active;
  atomic_int pending_messages;

  HandlerFunc handler_func;

  // Concurrent-safe, protected by its own lock
  FIFO mailbox;

  // Private actor state. Not protected by anything, as the scheduler
  // guarantees only a thread at a time will access it
  void *private;
} Actor;

void actor_init(Actor *actor, HandlerFunc handler_func, void *private);
bool actor_try_activating(Actor *actor);

// Scheduler

void scheduler_init();
PID scheduler_start(Actor *actor);

void scheduler_send(PID destination, Message *message);

#endif // CORE_H
