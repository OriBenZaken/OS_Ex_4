// Name: Ori Ben zaken , ID: 311492110


#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "osqueue.h"


#define FAILURE -1
#define SUCCESS 0
#define DONT_WAIT_FOR_TASKS 0

typedef struct thread_pool
{
     //The field x is here because a struct without fields
     //doesn't compile. Remove it once you add fields of your own
     int numOfThreads;
     pthread_t* threads;
     struct os_queue* tasksQueue;
     pthread_mutex_t lock;
     pthread_mutex_t queueLock;
     pthread_cond_t notify;
     int stopped;
     int canInsert;
}ThreadPool;

/**
 * creates a thread pool struct.
 * @param numOfThreads number of threads in the thread pool.
 * @return reference to new thread pool struct if succeeded, NULL if failed.
 */
ThreadPool* tpCreate(int numOfThreads);

/**
 * Destroys the thread pool.
 * @param threadPool thread pool
 * @param shouldWaitForTasks 0 - dont wait for tasks in the queue, else - wait for tasks.
 */
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

/**
 * inserts a task to the tasks queue of the thread pool.
 * @param threadPool thread pool
 * @param computeFunc task
 * @param param argument to the task
 * @return 0- success , -1 - fail
 */
int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
