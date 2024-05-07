#include <stdio.h>
#include <unistd.h>

#include "core.h"

static void basic_actor_handler(void *self, Message *message) {
  if (strcmp(message->name, "ping") == 0) {
    printf("PONG from a basic actor.\n");
  } else {
    printf("Got unknown message.\n");
  }
}

int main(int argc, char *argv[]) {
  scheduler_init();

  Actor basic_actor;
  actor_init(&basic_actor, basic_actor_handler, NULL);

  PID basic_actor_pid = scheduler_start(&basic_actor);

  Message msg = { .name = "ping" };

  while (1) {
    sleep(1);
    scheduler_send(basic_actor_pid, &msg);
  }
}
