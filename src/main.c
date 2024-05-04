#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "array.h"

#define MESSAGE_INIT 0
#define MESSAGE_PING 1


struct Actor;

typedef void (*handler_func)(struct Actor *);

typedef struct handler {
  int message;
  handler_func func;
} handler;

typedef array_t(handler) handler_array_t;

#define STATUS_IDLE 0      // Not running, and no messages in its mailbox
#define STATUS_RUNNABLE 1  // Not running, but has pending messages
#define STATUS_RUNNING 2   // Scheduled and currently running

typedef struct Actor {
  int id; // set by the scheduler
  atomic_int status;

  pthread_rwlock_t handlers_rwlock;
  handler_array_t handlers;
} Actor;

typedef struct {
  int id;
  pthread_t os_handle;
  struct Scheduler *scheduler;
} Thread;

typedef array_t(Actor *) actor_array_t;

typedef struct Scheduler {
  int num_threads;
  Thread *threads;

  int next_actor_id;

  pthread_mutex_t got_work_mutex;
  pthread_cond_t got_work_cond;

  pthread_rwlock_t known_actors_rwlock;
  actor_array_t known_actors;
} Scheduler;

static void *scheduler_worker(void *arg);

void scheduler_init(Scheduler *scheduler) {
  pthread_attr_t attr;
  if (pthread_attr_init(&attr) != 0) {
    perror("pthread_attr_init");
    exit(1);
  }

  scheduler->next_actor_id = 1;

  array_init(&scheduler->known_actors);

  if (pthread_rwlock_init(&scheduler->known_actors_rwlock, NULL) != 0) {
    perror("pthread_rwlock_init");
    exit(1);
  }

  // Run one thread per CPU core
  /* const int NUM_THREADS = sysconf(_SC_NPROCESSORS_ONLN); */
  const int NUM_THREADS = 2;

  scheduler->threads = calloc(NUM_THREADS, sizeof(Thread));
  assert(scheduler->threads != NULL);

  if (pthread_mutex_init(&scheduler->got_work_mutex, NULL) != 0) {
    perror("pthread_mutex_init");
    exit(1);
  }

  if (pthread_cond_init(&scheduler->got_work_cond, NULL) != 0) {
    perror("pthread_cond_init");
    exit(1);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    Thread *thread = &scheduler->threads[i];
    thread->id = i;
    thread->scheduler = scheduler;

    if (pthread_create(&thread->os_handle, &attr, &scheduler_worker, (void *)thread) != 0) {
      perror("pthread_create");
      exit(1);
    }

    scheduler->num_threads++;
  }
}

void scheduler_launch(Scheduler *scheduler, Actor *actor) {
  actor->id = scheduler->next_actor_id++;

  assert(pthread_rwlock_wrlock(&scheduler->known_actors_rwlock) == 0);
  array_push(&scheduler->known_actors, actor);
  pthread_rwlock_unlock(&scheduler->known_actors_rwlock);
}

static bool set_actor_status(Actor *actor, int from, int to) {
  return atomic_compare_exchange_strong(&actor->status, &from, to);
}

static handler_func lookup_handler(Actor *actor, int message) {
  handler_func ret = NULL;

  assert(pthread_rwlock_rdlock(&actor->handlers_rwlock) == 0);

  for (int i = 0; i < actor->handlers.length; i++) {
    struct handler *h = &actor->handlers.data[i];
    if (h->message == message) {
      ret = h->func;
      break;
    }
  }

  pthread_rwlock_unlock(&actor->handlers_rwlock);

  return ret;
}

void *scheduler_worker(void *arg) {
  Thread *thread = (Thread *)arg;
  Scheduler *scheduler = thread->scheduler;
  Actor *current_actor;

  while (1) {
    // Lock list of known actors, and find the first runnable one
    current_actor = NULL;

    assert(pthread_rwlock_rdlock(&scheduler->known_actors_rwlock) == 0);
    for (int i = 0; i < scheduler->known_actors.length; i++) {
      Actor *actor = scheduler->known_actors.data[i];
      if (actor->status != STATUS_RUNNABLE) {
        continue;
      }

      if (set_actor_status(actor, STATUS_RUNNABLE, STATUS_RUNNING)) {
        current_actor = actor;
        break;
      }
    }
    assert(pthread_rwlock_unlock(&scheduler->known_actors_rwlock) == 0);


    if (current_actor) {
      printf("Thread %d: Found an actor to run:\n", thread->id);

      handler_func handler;
      if ((handler = lookup_handler(current_actor, MESSAGE_INIT))) {
        handler(current_actor);
      } else {
        printf("BUG: Handler not found!\n");
      }

      assert(set_actor_status(current_actor, STATUS_RUNNING, STATUS_IDLE) == true);
    } else {
      printf("Thread %d: Found nothing to run. Going to sleep\n", thread->id);

      pthread_mutex_lock(&scheduler->got_work_mutex);
      assert(pthread_cond_wait(&scheduler->got_work_cond, &scheduler->got_work_mutex) == 0);
      pthread_mutex_unlock(&scheduler->got_work_mutex);
    }
  }
}

//

void actor_init(Actor *actor) {
  actor->status = STATUS_RUNNABLE;

  array_init(&actor->handlers);

  if (pthread_rwlock_init(&actor->handlers_rwlock, NULL) != 0) {
    perror("pthread_rwlock_init");
    exit(1);
  }
}

void actor_add_handler(Actor *actor, int message, void (*func)(struct Actor *)) {
  handler h;

  h.message = message;
  h.func = func;

  assert(pthread_rwlock_wrlock(&actor->handlers_rwlock) == 0);
  array_push(&actor->handlers, h);
  pthread_rwlock_unlock(&actor->handlers_rwlock);
}

static void basic_actor_init(Actor *self) {
  printf("Hello from a basic actor.\n");
}

//

int main(int argc, char *argv[]) {
  Scheduler scheduler;

  scheduler_init(&scheduler);

  Actor basic_actor;

  actor_init(&basic_actor);
  actor_add_handler(&basic_actor, MESSAGE_INIT, basic_actor_init);

  scheduler_launch(&scheduler, &basic_actor);

  while (1) {
    sleep(1);
  }
}
