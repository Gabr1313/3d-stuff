#pragma once

#include "../types.h"
#include "arena.cpp"
#include <pthread.h>
#include <semaphore.h>

struct Thread {
	void *(*fn)(void*);
	pthread_t handle;
	void     *args;
	sem_t     start;
	sem_t     finished;
	b1        stop; // TODO: should this be atomic?
};

void* _thread_loop(void *args) {
	Thread *t = (Thread*)args;
	while (!t->stop) {
		sem_wait(&t->start);
		if (t->fn != NULL) {
			t->fn(t->args);
		}
		sem_post(&t->finished);
	}
	return NULL;
};

Thread* thread_new(Arena *arena) {
	Thread *th = arena_push_struct_zero(arena, Thread);
	sem_init(&th->start   , 0, 0);
	sem_init(&th->finished, 0, 0);
    pthread_create(&th->handle, 0, &_thread_loop, th);
	return th;
}

void thread_stop(Thread *th) {
	th->stop = 1;
	th->fn = NULL;
	sem_post(&th->start);
	pthread_join(th->handle, NULL);
}

void thread_start(Thread *th, void *(*fn)(void*), void *args) {
	th->fn   = fn;
	th->args = args;
	sem_post(&th->start);
}

void thread_wait(Thread *th) {
	sem_wait(&th->finished);
}
