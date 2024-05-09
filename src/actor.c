#include <stdio.h>

#include "core.h"

void actor_init(Actor *actor, HandlerFunc handler_func, void *private) {
  actor->is_active = false;
  actor->handler_func = handler_func;
  actor->private = private;
  fifo_init(&actor->mailbox);
}

// Atomically activate an actor. Returns true if operation succeeds.
bool actor_acquire(Actor *actor) {
  bool expected_active = false;
  return atomic_compare_exchange_strong(&actor->is_active, &expected_active, true);
}
