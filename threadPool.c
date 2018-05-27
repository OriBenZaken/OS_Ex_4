//
// Created by ori on 5/26/18.
//

#include "threadPool.h"

typedef struct task
{
    void (*computeFunc)(void *param);
    void* param;
}Task;

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
    tp->canInsert = 0;

    int i, err;
    for (i = 0; i < tp->numOfThreads; i++) {
        err = pthread_create(&(tp->threads[i]), NULL, execute, (void *)tp);
    }

    return tp;
}

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param) {
    if (!(threadPool->canInsert)) {
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

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
    threadPool->canInsert = 0;

    if (shouldWaitForTasks) {
        while (!osIsQueueEmpty(threadPool->tasksQueue));
        int i, err;
        for (i = 0; i < threadPool->numOfThreads; i++) {
            err = pthread_join(threadPool->threads[i], NULL);
            if (err != 0) {
                printf("Failure: waiting for thread no. %d\n", i);
            }
        }
    }

    threadPool->stopped = 1;

    //free memory
}
