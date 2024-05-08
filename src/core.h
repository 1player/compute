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
void mailbox_push(Mailbox *mailbox, Message message);
bool mailbox_pop(Mailbox *mailbox, Message *out);

// Actor

typedef unsigned int PID;

typedef void (*HandlerFunc)(void *, Message);

typedef struct Actor {
  PID pid;

  atomic_bool is_active;
  atomic_int pending_messages;

  HandlerFunc handler_func;

  // Concurrent-safe, protected by its own lock
  Mailbox mailbox;

  // Private actor state. Not protected by anything, as the scheduler
  // guarantees only a thread at a time will access it
  void *private;
} Actor;

void actor_init(Actor *actor, HandlerFunc handler_func, void *private);
bool actor_try_activating(Actor *actor);

// Scheduler

void scheduler_init();
PID scheduler_start(Actor *actor);

void scheduler_send(PID destination, Message message);

#endif // CORE_H
