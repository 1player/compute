#include <stdio.h>
#include <unistd.h>

#include "core.h"

static void basic_actor_handler(void *self) {
  printf("Hello from a basic actor.\n");
}

int main(int argc, char *argv[]) {
  Actor basic_actor;

  scheduler_init();

  actor_init(&basic_actor, basic_actor_handler, NULL, NULL);

  scheduler_start(&basic_actor);

  while (1) {
    sleep(1);
  }
}
