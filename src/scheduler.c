#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <threads.h>

#include "core.h"

typedef array_t(Actor *) actor_array_t;

typedef struct {
  int id;
  pthread_t os_handle;
} SchedulerThread;

typedef struct Scheduler {
  int num_threads;
  SchedulerThread *threads;

  atomic_int next_actor_id;

  pthread_mutex_t got_work_mutex;
  pthread_cond_t got_work_cond;

  pthread_rwlock_t known_actors_rwlock;
  actor_array_t known_actors;
} Scheduler;

Scheduler global_scheduler;
FIFO global_runqueue;
thread_local Actor *local_current_actor = NULL;

#define LOCK_ACTORS_READ assert(pthread_rwlock_rdlock(&global_scheduler.known_actors_rwlock) == 0)
#define LOCK_ACTORS_WRITE assert(pthread_rwlock_wrlock(&global_scheduler.known_actors_rwlock) == 0)
#define UNLOCK_ACTORS pthread_rwlock_unlock(&global_scheduler.known_actors_rwlock);

static void notify_got_work() {
  pthread_mutex_lock(&global_scheduler.got_work_mutex);
  pthread_cond_signal(&global_scheduler.got_work_cond);
  pthread_mutex_unlock(&global_scheduler.got_work_mutex);
}

static void wait_for_work() {
  pthread_mutex_lock(&global_scheduler.got_work_mutex);
  assert(pthread_cond_wait(&global_scheduler.got_work_cond, &global_scheduler.got_work_mutex) == 0);
  pthread_mutex_unlock(&global_scheduler.got_work_mutex);
}

static void thread_run_actor(SchedulerThread *thread, Actor *actor) {
  Message *msg;

  // Set thread local pointing to current actor
  local_current_actor = actor;

  while ((msg = fifo_pop(&actor->mailbox))) {
    printf("Thread %d: Dispatching '%s' to PID %d:\n", thread->id, msg->name, actor->pid);

    for (int i = 0; i < actor->handlers->count; i++) {
      HandlerEntry *entry = &actor->handlers->entries[i];
      if (strcmp(entry->name, msg->name) != 0) {
        continue;
      }

      entry->handler(actor->private, msg);
    }
  }

  printf("Thread %d: Done with PID %d.\n", thread->id, actor->pid);
  actor->is_active = false;
}

static void *thread_main(void *arg) {
  SchedulerThread *thread = (SchedulerThread *)arg;
  Actor *actor;

  while (1) {
    // Pick actor from the global runqueue, and try to acquire it
    do {
      actor = fifo_pop(&global_runqueue);
    } while (actor && !actor_acquire(actor));

    if (actor) {
      thread_run_actor(thread, actor);
    } else {
      /* printf("Thread %d: Going to sleep.\n", thread->id); */
      wait_for_work();
    }
  }

  return NULL;
}


void scheduler_init() {
  pthread_attr_t attr;
  if (pthread_attr_init(&attr) != 0) {
    perror("pthread_attr_init");
    exit(1);
  }

  fifo_init(&global_runqueue);

  // Init the global scheduler
  global_scheduler.next_actor_id = 1;
  array_init(&global_scheduler.known_actors);
  assert(pthread_rwlock_init(&global_scheduler.known_actors_rwlock, NULL) == 0);
  assert(pthread_mutex_init(&global_scheduler.got_work_mutex, NULL) == 0);
  assert(pthread_cond_init(&global_scheduler.got_work_cond, NULL) == 0);

  // Run one thread per CPU core
  global_scheduler.num_threads = sysconf(_SC_NPROCESSORS_ONLN);
  global_scheduler.threads = calloc(global_scheduler.num_threads, sizeof(SchedulerThread));

  for (int i = 0; i < global_scheduler.num_threads; i++) {
    SchedulerThread *thread = &global_scheduler.threads[i];
    thread->id = i;

    // Start from thread #1, thread #0 will be jumped into from the main function.
    if (i == 0) {
      continue;
    }

    assert(pthread_create(&thread->os_handle, &attr, &thread_main, (void *)thread) == 0);
  }
}

PID next_pid() {
  return global_scheduler.next_actor_id++;
}

PID scheduler_start(void *private, HandlerTable *handlers) {
  Actor *actor = actor_new();
  actor->private = private;
  actor->handlers = handlers;
  actor->pid = next_pid();

  LOCK_ACTORS_WRITE;
  array_push(&global_scheduler.known_actors, actor);
  UNLOCK_ACTORS;

  Message *init_message = malloc(sizeof(Message));
  init_message->name = "init";
  scheduler_cast(actor->pid, init_message);

  return actor->pid;
}


void scheduler_absorb_main_thread() {
  SchedulerThread *main_thread = &global_scheduler.threads[0];
  thread_main(main_thread);

  // unreachable
}

PID scheduler_self() {
  if (local_current_actor) {
    return local_current_actor->pid;
  }

  return -1;
}

static Actor *lookup_pid(PID pid) {
  Actor *found_actor = NULL;

  LOCK_ACTORS_READ;
  
  for (int i = 0; i < global_scheduler.known_actors.length; i++) {
    Actor *actor = global_scheduler.known_actors.data[i];

    if (actor->pid == pid) {
      found_actor = actor;
      break;
    }
  }

  UNLOCK_ACTORS;
  return found_actor;
}

static void add_actor_to_runqueue(Actor *actor) {
  fifo_push(&global_runqueue, (void *)actor);
  notify_got_work();
}

void scheduler_cast(PID pid, Message *message) {
  Actor *actor = lookup_pid(pid);
  if (!actor) {
    printf("Trying to send message to unknown actor.\n");
    return;
  }

  message->sender = scheduler_self();
  fifo_push(&actor->mailbox, (void *)message);

  // TODO: possible race condition when actor goes inactive
  if (!actor->is_active) {
    add_actor_to_runqueue(actor);
  }
}
