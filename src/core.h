#ifndef CORE_H
#define CORE_H

#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

#include "array.h"

// Actor

typedef unsigned int PID;

#define STATUS_IDLE 0      // Not running, and no messages in its mailbox
#define STATUS_RUNNABLE 1  // Not running, but has pending messages
#define STATUS_RUNNING 2   // Scheduled and currently running

typedef struct Actor {
  PID pid; // set by the scheduler
  atomic_int status;

  void (*handler_func)(void *);

  void *public;
  void *private;
} Actor;

void actor_init(Actor *actor, void (*handler_func)(void *), void *private, void *public);
bool actor_set_status(Actor *actor, int from, int to);

// Scheduler

typedef array_t(Actor *) actor_array_t;

typedef struct {
  int id;
  pthread_t os_handle;
  struct Scheduler *scheduler;
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

void scheduler_init(Scheduler *scheduler);
void scheduler_start(Scheduler *scheduler, Actor *actor);

#endif // CORE_H
