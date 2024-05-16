#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

#include "lib/lib.h"

typedef unsigned int PID;

// Message

typedef struct Message {
  char *name;
  PID sender;
  void *data;
} Message;

// Actor

typedef void (*HandlerFunc)(void *, const Message *);

typedef struct HandlerEntry {
  char *name;
  HandlerFunc handler;
} HandlerEntry;

typedef struct HandlerTable {
  int count;
  HandlerEntry entries[];
} HandlerTable;

typedef struct Actor {
  PID pid;
  atomic_bool is_active;
  HandlerTable *handlers;

  // Concurrent-safe, protected by its own lock
  FIFO mailbox;

  // Private actor state. Not protected by anything, as the scheduler
  // guarantees only a thread at a time will access it
  void *private;
} Actor;

Actor *actor_new();
bool actor_acquire(Actor *actor);

// Scheduler

void scheduler_init();
PID scheduler_start(void *private, HandlerTable *handlers);
void scheduler_absorb_main_thread();

void scheduler_cast(PID destination, Message *message);
PID scheduler_self();

#endif // CORE_H
