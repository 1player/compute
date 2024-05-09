#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "core.h"

typedef array_t(Actor *) actor_array_t;

typedef struct {
  int id;
  pthread_t os_handle;
} SchedulerThread;

typedef struct Scheduler {
  int num_threads;
  SchedulerThread *threads;

  int next_actor_id;

  pthread_mutex_t got_work_mutex;
  pthread_cond_t got_work_cond;

  pthread_rwlock_t known_actors_rwlock;
  actor_array_t known_actors;
} Scheduler;

Scheduler global_scheduler;

#define LOCK_ACTORS_READ assert(pthread_rwlock_rdlock(&global_scheduler.known_actors_rwlock) == 0)
#define LOCK_ACTORS_WRITE assert(pthread_rwlock_wrlock(&global_scheduler.known_actors_rwlock) == 0)
#define UNLOCK_ACTORS pthread_rwlock_unlock(&global_scheduler.known_actors_rwlock);


static Actor *find_actor_to_run() {
  Actor *actor_to_run = NULL;

  LOCK_ACTORS_READ;

  for (int i = 0; i < global_scheduler.known_actors.length; i++) {
    Actor *actor = global_scheduler.known_actors.data[i];
    if (actor->pending_messages < 1) {
      continue;
    }

    if (actor_try_activating(actor)) {
      actor_to_run = actor;
      break;
    }
  }

  UNLOCK_ACTORS;

  return actor_to_run;
}

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

static void *thread_main(void *arg) {
  SchedulerThread *thread = (SchedulerThread *)arg;
  Actor *current_actor;

  while (1) {
    current_actor = find_actor_to_run();

    if (current_actor) {
      printf("Thread %d: Found actor %p to run.\n", thread->id, current_actor);

      Message *msg = fifo_pop(&current_actor->mailbox);

      if (msg) {
        printf("Thread %d: Dispatching '%s' to %p:\n", thread->id, msg->name, current_actor);
        current_actor->pending_messages--;
        current_actor->handler_func(current_actor->private, msg);
      }
      printf("Thread %d: Done with %p.\n", thread->id, current_actor);

      current_actor->is_active = false;
    } else {
      printf("Thread %d: Going to sleep.\n", thread->id);

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

  global_scheduler.next_actor_id = 1;

  array_init(&global_scheduler.known_actors);

  if (pthread_rwlock_init(&global_scheduler.known_actors_rwlock, NULL) != 0) {
    perror("pthread_rwlock_init");
    exit(1);
  }

  // Run one thread per CPU core
  const int NUM_THREADS = sysconf(_SC_NPROCESSORS_ONLN);

  global_scheduler.threads = calloc(NUM_THREADS, sizeof(SchedulerThread));
  assert(global_scheduler.threads != NULL);

  if (pthread_mutex_init(&global_scheduler.got_work_mutex, NULL) != 0) {
    perror("pthread_mutex_init");
    exit(1);
  }

  if (pthread_cond_init(&global_scheduler.got_work_cond, NULL) != 0) {
    perror("pthread_cond_init");
    exit(1);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    SchedulerThread *thread = &global_scheduler.threads[i];
    thread->id = i;

    if (pthread_create(&thread->os_handle, &attr, &thread_main, (void *)thread) != 0) {
      perror("pthread_create");
      exit(1);
    }

    global_scheduler.num_threads++;
  }
}

PID scheduler_start(Actor *actor) {
  PID actor_pid = global_scheduler.next_actor_id++;
  actor->pid = actor_pid;

  LOCK_ACTORS_WRITE;
  array_push(&global_scheduler.known_actors, actor);
  UNLOCK_ACTORS;

  Message *init_message = malloc(sizeof(Message));
  init_message->name = "init";
  scheduler_send(actor_pid, init_message);

  return actor_pid;
}

// Assumes actors are locked
static Actor *lookup_pid(PID pid) {
  for (int i = 0; i < global_scheduler.known_actors.length; i++) {
    Actor *actor = global_scheduler.known_actors.data[i];

    if (actor->pid == pid)
      return actor;
  }

  return NULL;
}


void scheduler_send(PID pid, Message *message) {
  LOCK_ACTORS_READ;

  Actor *actor = lookup_pid(pid);
  if (!actor) {
    printf("Trying to send message to unknown actor.\n");
    goto end;
  }

  fifo_push(&actor->mailbox, (void *)message);
  actor->pending_messages++;

  notify_got_work();

 end:
  UNLOCK_ACTORS;
}
