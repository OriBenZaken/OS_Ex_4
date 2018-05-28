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

    while (!tp->stopped && !(tp->canInsert == 0 && osIsQueueEmpty(tp->tasksQueue))) {
//        if (osIsQueueEmpty(tp->tasksQueue)) {
//            //printf("locked\n");
//            pthread_mutex_lock(&(tp->queueLock));
//        }
        pthread_mutex_lock(&(tp->lock));
        if (!(osIsQueueEmpty(taskQueue))) {
            Task* task = osDequeue(taskQueue);
            pthread_mutex_unlock(&(tp->lock));
            task->computeFunc(task->param);
            free(task);
        }
        else {
            pthread_mutex_unlock(&(tp->lock));
            //pthread_mutex_lock(&(tp->queueLock));
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
    tp->canInsert = 1;
    pthread_mutex_init(&(tp->queueLock), NULL);
    //pthread_mutex_lock(&(tp->queueLock));

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
    int queueWasEmpty = 0;
    if (osIsQueueEmpty(threadPool->tasksQueue)) {
        queueWasEmpty = 1;
    }

    osEnqueue(threadPool->tasksQueue, (void *)task);

//    if (queueWasEmpty) {
//        pthread_mutex_unlock(&(threadPool->queueLock));
//    }
    return SUCCESS;
}

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
    threadPool->canInsert = 0;

    if (shouldWaitForTasks == 0) {
        threadPool->stopped = 1;
    }
    int i, err;
    for (i = 0; i < threadPool->numOfThreads; i++) {
        err = pthread_join(threadPool->threads[i], NULL);
        if (err != 0) {
            printf("Failure: waiting for thread no. %d\n", i);
        }
    }


    threadPool->stopped = 1;

    while (!osIsQueueEmpty(threadPool->tasksQueue)) {
        printf("Task was erased from tasks queu\n");
        Task* task = osDequeue(threadPool->tasksQueue);
        free(task);
    }

    osDestroyQueue(threadPool->tasksQueue);
    free(threadPool->threads);
    pthread_mutex_destroy(&(threadPool->lock));
    //pthread_mutex_destroy(&(threadPool->queueLock));
    free(threadPool);


    //free memory
}
