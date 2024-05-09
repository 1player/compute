#ifndef CORE_H
#define CORE_H

#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

#include "array.h"

typedef unsigned int PID;

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
  PID sender;
} Message;

// Actor

typedef void (*HandlerFunc)(void *, Message *);

typedef struct Actor {
  PID pid;

  atomic_bool is_active;

  HandlerFunc handler_func;

  // Concurrent-safe, protected by its own lock
  FIFO mailbox;

  // Private actor state. Not protected by anything, as the scheduler
  // guarantees only a thread at a time will access it
  void *private;
} Actor;

void actor_init(Actor *actor, HandlerFunc handler_func, void *private);
bool actor_acquire(Actor *actor);

// Scheduler

void scheduler_init();
PID scheduler_start(Actor *actor);
void scheduler_absorb_main_thread();

void scheduler_cast(PID destination, Message *message);
PID scheduler_self();

#endif // CORE_H
