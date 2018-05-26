#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "osqueue.h"


#define FAILURE 0
#define SUCCESS 1

typedef struct thread_pool
{
     //The field x is here because a struct without fields
     //doesn't compile. Remove it once you add fields of your own
     int numOfThreads;
     pthread_t* threads;
     struct os_queue* tasksQueue;
     pthread_mutex_t lock;
     int stopped;
}ThreadPool;

typedef struct task
{
    void (*computeFunc)(void *param);
    void* param;
}Task;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
