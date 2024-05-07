#ifndef CORE_H
#define CORE_H

#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

#include "array.h"

// Message

typedef struct Message {
  char *name;
} Message;

// Mailbox

struct MailboxEntry;

typedef struct Mailbox {
  pthread_mutex_t lock;
  struct MailboxEntry *front;
  struct MailboxEntry *back;
} Mailbox;

void mailbox_init(Mailbox *mailbox);
void mailbox_push(Mailbox *mailbox, Message *message);
Message *mailbox_pop(Mailbox *mailbox);

// Actor

typedef unsigned int PID;

#define STATUS_IDLE 0      // Not running, and no messages in its mailbox
#define STATUS_RUNNABLE 1  // Not running, but has pending messages
#define STATUS_RUNNING 2   // Scheduled and currently running

typedef void (*HandlerFunc)(void *, Message *);

typedef struct Actor {
  PID pid; // set by the scheduler
  atomic_int status;

  HandlerFunc handler_func;

  // Concurrent-safe, protected by its own lock
  Mailbox mailbox;

  // Private actor state. Not protected by anything, as the scheduler
  // guarantees only a thread at a time will access it
  void *private;
} Actor;

void actor_init(Actor *actor, HandlerFunc handler_func, void *private);
bool actor_set_status(Actor *actor, int from, int to);

// Scheduler

void scheduler_init();
PID scheduler_start(Actor *actor);

void scheduler_send(PID destination, Message *message);

#endif // CORE_H
