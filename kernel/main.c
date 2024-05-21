#include <stdio.h>
#include <unistd.h>

#include "kernel.h"

struct sieve {
  int n;
  PID child;
};

HandlerTable sieve_handlers;

static void sieve_init(struct sieve *self, const Message *msg) {
  printf("%d\n", self->n);
}

static void sieve_filter(struct sieve *self, const Message *msg) {
  int number = (int)msg->data;

  if ((number % self->n) == 0) {
    // Filtered by ourselves
    return;
  }

  // Not our multiple. Forward to our child.
  if (self->child) {
    Message *new_msg = malloc(sizeof(Message));
    new_msg->name = "filter";
    new_msg->data = (void *)number;
    scheduler_cast(self->child, new_msg);
    return;
  }

  // Start a new child if we have none
  struct sieve *child_sieve = calloc(1, sizeof(struct sieve));
  child_sieve->n = number;
  self->child = scheduler_start(child_sieve, &sieve_handlers);
}

HandlerTable sieve_handlers = {
  .count = 2,
  .entries = {
    { .name = "init", .handler = (HandlerFunc)sieve_init },
    { .name = "filter", .handler = (HandlerFunc)sieve_filter },
  }
};

int main(int argc, char *argv[]) {
  scheduler_init();

  struct sieve two_sieve = { .n = 2, .child = 0 };
  PID two_sieve_pid = scheduler_start(&two_sieve, &sieve_handlers);

  for (int i = 3; i < 10000; i++) {
    Message *msg = malloc(sizeof(Message));
    msg->name = "filter";
    msg->data = (void *)i;
    scheduler_cast(two_sieve_pid, msg);
  }

  // Turn this main thread into another scheduler thread
  scheduler_absorb_main_thread();

  // unreachable
}
