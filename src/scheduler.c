#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "core.h"

static void *scheduler_worker(void *arg) {
  SchedulerThread *thread = (SchedulerThread *)arg;
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

      if (actor_set_status(actor, STATUS_RUNNABLE, STATUS_RUNNING)) {
        current_actor = actor;
        break;
      }
    }
    assert(pthread_rwlock_unlock(&scheduler->known_actors_rwlock) == 0);


    if (current_actor) {
      printf("Thread %d: Found an actor to run:\n", thread->id);

      current_actor->handler_func(current_actor->private);

      assert(actor_set_status(current_actor, STATUS_RUNNING, STATUS_IDLE) == true);
    } else {
      printf("Thread %d: Found nothing to run. Going to sleep\n", thread->id);

      pthread_mutex_lock(&scheduler->got_work_mutex);
      assert(pthread_cond_wait(&scheduler->got_work_cond, &scheduler->got_work_mutex) == 0);
      pthread_mutex_unlock(&scheduler->got_work_mutex);
    }
  }
}

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
  const int NUM_THREADS = sysconf(_SC_NPROCESSORS_ONLN);

  scheduler->threads = calloc(NUM_THREADS, sizeof(SchedulerThread));
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
    SchedulerThread *thread = &scheduler->threads[i];
    thread->id = i;
    thread->scheduler = scheduler;

    if (pthread_create(&thread->os_handle, &attr, &scheduler_worker, (void *)thread) != 0) {
      perror("pthread_create");
      exit(1);
    }

    scheduler->num_threads++;
  }
}

void scheduler_start(Scheduler *scheduler, Actor *actor) {
  actor->pid = scheduler->next_actor_id++;

  assert(pthread_rwlock_wrlock(&scheduler->known_actors_rwlock) == 0);
  array_push(&scheduler->known_actors, actor);
  pthread_rwlock_unlock(&scheduler->known_actors_rwlock);
}
