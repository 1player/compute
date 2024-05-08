#include <stdio.h>

#include "core.h"

void actor_init(Actor *actor, HandlerFunc handler_func, void *private) {
  actor->is_active = false;
  actor->pending_messages = 0;
  actor->handler_func = handler_func;
  actor->private = private;
  mailbox_init(&actor->mailbox);
}

bool actor_try_activating(Actor *actor) {
  bool expected_active = false;
  return atomic_compare_exchange_strong(&actor->is_active, &expected_active, true);
}
