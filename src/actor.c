#include <stdio.h>

#include "core.h"

void actor_init(Actor *actor, HandlerFunc handler_func, void *private) {
  actor->status = STATUS_IDLE;
  actor->handler_func = handler_func;
  actor->private = private;
  mailbox_init(&actor->mailbox);
}

bool actor_set_status(Actor *actor, int from, int to) {
  return atomic_compare_exchange_strong(&actor->status, &from, to);
}
