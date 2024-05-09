#include <stdio.h>
#include <unistd.h>

#include "core.h"

struct basic_actor {
  PID partner;
};

static void basic_actor_init(struct basic_actor *self) {
  if (self->partner) {
    printf("PID %d: Pinging partner...\n", scheduler_self());

    Message *msg = malloc(sizeof(Message));
    msg->name = "ping";
    scheduler_cast(self->partner, msg);
  }
}

static void basic_actor_ping(struct basic_actor *self, PID sender) {
  if (self->partner) {
    printf("PID %d: Didn't expect a ping ourselves...\n", scheduler_self());
  } else {
    printf("PID %d: Got a ping from PID %d!\n", scheduler_self(), sender);
  }
}
  

static void basic_actor_handler(struct basic_actor *self, Message *message) {
  if (strcmp(message->name, "init") == 0) {
    printf("PID %d initialized.\n", scheduler_self());
    basic_actor_init(self);
  } else if (strcmp(message->name, "ping") == 0) {
    basic_actor_ping(self, message->sender);
  } else {
    printf("Got unknown message.\n");
  }

  free(message);
}

int main(int argc, char *argv[]) {
  scheduler_init();

  Actor actor1;
  struct basic_actor actor1_data = { 0 };
  actor_init(&actor1, (HandlerFunc)basic_actor_handler, &actor1_data);
  PID actor1_pid = scheduler_start(&actor1);

  Actor actor2;
  struct basic_actor actor2_data = { .partner = actor1_pid };
  actor_init(&actor2, (HandlerFunc)basic_actor_handler, &actor2_data);
  PID actor2_pid = scheduler_start(&actor2);

  // Turn this main thread into another scheduler thread
  scheduler_absorb_main_thread();

  // unreachable
}
