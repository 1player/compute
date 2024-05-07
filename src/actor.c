#include "core.h"

void actor_init(Actor *actor, void (*handler_func)(void *), void *public, void *private) {
  actor->status = STATUS_RUNNABLE;
  actor->handler_func = handler_func;
  actor->public = NULL;
  actor->private = private;
}

bool actor_set_status(Actor *actor, int from, int to) {
  return atomic_compare_exchange_strong(&actor->status, &from, to);
}
