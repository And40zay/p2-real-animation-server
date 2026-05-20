#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>

typedef struct threadpool threadpool_t;

threadpool_t* threadpool_create(int num_threads);
void threadpool_destroy(threadpool_t* pool);
int threadpool_add_task(threadpool_t* pool, void (*function)(void*), void* arg);

#endif