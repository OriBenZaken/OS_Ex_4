//
// Created by ori on 5/26/18.
//

#include "threadPool.h"

void* execute(void* args) {
    ThreadPool* tp = (ThreadPool*)args;
    struct os_queue* taskQueue = tp->tasksQueue;
    printf("New thread was created\n");

    while (!tp->stopped) {
        pthread_mutex_lock(&(tp->lock));
        if (!(osIsQueueEmpty(taskQueue))) {
            Task* task = osDequeue(taskQueue);
            pthread_mutex_unlock(&(tp->lock));
            task->computeFunc(task->param);
        }
        else {
            pthread_mutex_unlock(&(tp->lock));
            sleep(1);
        }
    }
}

ThreadPool* tpCreate(int numOfThreads) {
    ThreadPool* tp = (ThreadPool*)malloc(sizeof(ThreadPool));
    if (tp == NULL) {
        printf("Failure: allocate memory for thread pool struct");
        exit(1);
    }
    tp->numOfThreads = numOfThreads;

    tp->threads = (pthread_t*)malloc(sizeof(pthread_t) * tp->numOfThreads);
    if (tp->threads == NULL) {
        printf("Failure: allocate memory for threads array");
        exit(1);
    }

    tp->tasksQueue = osCreateQueue();
    pthread_mutex_init(&(tp->lock), NULL);
    tp->stopped = 0;

    int i, err;
    for (i = 0; i < tp->numOfThreads; i++) {
        err = pthread_create(&(tp->threads[i]), NULL, execute, (void *)tp);
    }

    return tp;
}

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param) {
    if (threadPool->stopped) {
        return FAILURE;
    }

    Task* task = (Task*)malloc(sizeof(Task));
    if (task == NULL) {
        printf("Failure: allocate memory for threads array");
        exit(1);
    }

    task->computeFunc = computeFunc;
    task->param = param;

    osEnqueue(threadPool->tasksQueue, (void *)task);
    return SUCCESS;
}


