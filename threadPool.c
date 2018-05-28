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

    /* Lock must be taken to wait on conditional variable */
    pthread_mutex_lock(&(tp->queueLock));

    /* Wait on condition variable, check for spurious wakeups.
       When returning from pthread_cond_wait(), we own the lock. */
    if((osIsQueueEmpty(taskQueue)) && (!tp->stopped)) {
        printf("Busy waiting\n");
        pthread_cond_wait(&(tp->notify), &(tp->queueLock));
    }
    pthread_mutex_unlock(&(tp->queueLock));

    while (!tp->stopped && !(tp->canInsert == 0 && osIsQueueEmpty(taskQueue))) {
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
            //pthread_mutex_unlock(&(tp->queueLock));
        }
        else {
            pthread_mutex_unlock(&(tp->lock));
            //sleep(1);
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

    //todo: check success
    pthread_mutex_init(&(tp->queueLock), NULL);
    //pthread_mutex_lock(&(tp->queueLock));
    pthread_cond_init(&(tp->notify), NULL);

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

    pthread_mutex_lock(&(threadPool->queueLock));
    if(pthread_cond_signal(&(threadPool->notify)) != 0) {
        //todo: print error
    }
    pthread_mutex_unlock(&(threadPool->queueLock));
//    if(pthread_mutex_unlock(&threadPool->queueLock) != 0) {
//        //todo: print error
//    }

    return SUCCESS;
}

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
    threadPool->canInsert = 0;



    if (shouldWaitForTasks == 0) {
        threadPool->stopped = 1;
    }
    int i, err;

    if(pthread_mutex_lock(&(threadPool->queueLock)) != 0) {
        //todo: print err
    }
    /* Wake up all worker threads */
    if((pthread_cond_broadcast(&(threadPool->notify)) != 0) ||
       (pthread_mutex_unlock(&(threadPool->queueLock)) != 0)) {
        //todo: print err
        printf("Exit due failure in tpDestory\n");
        exit(1);
    }

    for (i = 0; i < threadPool->numOfThreads; i++) {
        err = pthread_join(threadPool->threads[i], NULL);
        if (err != 0) {
            printf("Failure: waiting for thread no. %d\n", i);
        }
    }


    threadPool->stopped = 1;

    while (!osIsQueueEmpty(threadPool->tasksQueue)) {
        printf("Task was erased from tasks queue\n");
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
