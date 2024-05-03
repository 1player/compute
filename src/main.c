#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <semaphore.h>

typedef struct {
  int id;
} Actor;

typedef struct {
  int id;
  pthread_t os_handle;
  struct scheduler *scheduler;
} Thread;

typedef struct scheduler {
  int num_threads;
  Thread *threads;

  sem_t semaphore;
} Scheduler;

static void *scheduler_worker(void *arg);

void scheduler_init(Scheduler *scheduler) {
  pthread_attr_t attr;
  if (pthread_attr_init(&attr) != 0) {
    perror("pthread_attr_init");
    exit(1);
  }

  // TODO: should be one thread per core
  const int NUM_THREADS = 4;

  scheduler->threads = calloc(NUM_THREADS, sizeof(Thread));
  assert(scheduler->threads != NULL);

  if (sem_init(&scheduler->semaphore, 0, 0) != 0) {
    perror("sem_init");
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

static void *scheduler_worker(void *arg) {
  Thread *thread = (Thread *)arg;
  Scheduler *scheduler = thread->scheduler;

  while (1) {
    assert(sem_wait(&scheduler->semaphore) == 0);

    printf("Hello from thread %d\n", thread->id);
  }
}

//

static void basic_actor(Actor *self) {

}

//

int main(int argc, char *argv[]) {
  Scheduler scheduler;

  scheduler_init(&scheduler);

  while (1) {
    sem_post(&scheduler.semaphore);
    sem_post(&scheduler.semaphore);
    sleep(1);
  }
}
