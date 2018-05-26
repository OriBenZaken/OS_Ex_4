#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include "osqueue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct thread_pool
{
     //The field x is here because a struct without fields
     //doesn't compile. Remove it once you add fields of your own
     int numOfThreads;
     pthread_t* threads;
     struct os_queue* tasksQueue;
     pthread_mutex_t lock;
}ThreadPool;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
