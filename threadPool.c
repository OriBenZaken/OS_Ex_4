//
// Created by ori on 5/26/18.
//

#include "threadPool.h"

void* execute(void* args) {
    ThreadPool* tp = (ThreadPool*)args;
    printf("New thread was created\n");
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
    pthread_mutex_init(&(tp->lock), PTHREAD_MUTEX_ERRORCHECK);
    
    int i, err;
    for (i = 0; i < tp->numOfThreads; i++) {
        err = pthread_create(&(tp->threads[i]), NULL, execute, (void *)tp);
    }
    return tp;
}


