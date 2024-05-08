#include <stdio.h>
#include <unistd.h>

#include "core.h"

struct basic_actor {
  PID partner;
};

static void basic_actor_init(struct basic_actor *self) {
  if (self->partner) {
    printf("Pinging partner...\n");

    Message msg = { .name = "ping" };
    scheduler_send(self->partner, msg);
  }
}

static void basic_actor_ping(struct basic_actor *self) {
  if (self->partner) {
    printf("Didn't expect a ping ourselves...\n");
  } else {
    printf("Got a ping!\n");
  }
}
  

static void basic_actor_handler(struct basic_actor *self, Message message) {
  if (strcmp(message.name, "init") == 0) {
    basic_actor_init(self);
  } else if (strcmp(message.name, "ping") == 0) {
    basic_actor_ping(self);
  } else {
    printf("Got unknown message.\n");
  }
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

  while(1) {
    sleep(1000);
  }
}
