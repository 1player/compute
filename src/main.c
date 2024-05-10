#include <stdio.h>
#include <unistd.h>

#include "core.h"

struct basic_actor {
  PID partner;
};

static void basic_actor_init(struct basic_actor *self, Message *msg) {
  if (self->partner) {
    printf("PID %d: Pinging partner...\n", scheduler_self());

    Message *msg = malloc(sizeof(Message));
    msg->name = "ping";
    scheduler_cast(self->partner, msg);
  }
}

static void basic_actor_ping(struct basic_actor *self, Message *msg) {
  if (self->partner) {
    printf("PID %d: Didn't expect a ping ourselves...\n", scheduler_self());
  } else {
    printf("PID %d: Got a ping from PID %d!\n", scheduler_self(), msg->sender);
  }
}

HandlerTable basic_actor_handlers = {
  .count = 2,
  .entries = {
    { .name = "init", .handler = (HandlerFunc)basic_actor_init },
    { .name = "ping", .handler = (HandlerFunc)basic_actor_ping },
  }
};

int main(int argc, char *argv[]) {
  scheduler_init();

  struct basic_actor actor1_data = { 0 };
  PID actor1_pid = scheduler_start(&actor1_data, &basic_actor_handlers);

  struct basic_actor actor2_data = { .partner = actor1_pid };
  PID actor2_pid = scheduler_start(&actor2_data, &basic_actor_handlers);

  // Turn this main thread into another scheduler thread
  scheduler_absorb_main_thread();

  // unreachable
}
