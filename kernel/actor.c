#include <stdio.h>

#include "scheduler.h"

Actor *actor_new() {
  Actor *actor = malloc(sizeof(Actor));
  actor->is_active = false;
  fifo_init(&actor->mailbox);

  return actor;
}

// Atomically activate an actor. Returns true if operation succeeds.
bool actor_acquire(Actor *actor) {
  bool expected_active = false;
  return atomic_compare_exchange_strong(&actor->is_active, &expected_active, true);
}
