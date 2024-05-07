#ifndef CORE_H
#define CORE_H

#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

#include "array.h"

// Actor

typedef unsigned int PID;

#define STATUS_IDLE 0      // Not running, and no messages in its mailbox
#define STATUS_RUNNABLE 1  // Not running, but has pending messages
#define STATUS_RUNNING 2   // Scheduled and currently running

typedef struct Actor {
  PID pid; // set by the scheduler
  atomic_int status;

  void (*handler_func)(void *);

  void *public;
  void *private;
} Actor;

void actor_init(Actor *actor, void (*handler_func)(void *), void *private, void *public);
bool actor_set_status(Actor *actor, int from, int to);

// Scheduler


void scheduler_init();
void scheduler_start(Actor *actor);

#endif // CORE_H
