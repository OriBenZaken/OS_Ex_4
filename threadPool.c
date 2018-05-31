// Name: Ori Ben zaken , ID: 311492110

#include <fcntl.h>
#include "threadPool.h"
#define STDERR_FD 2
#define SYS_CALL_FAILURE 10

pthread_mutex_t destryLock;

typedef struct task
{
    void (*computeFunc)(void *param);
    void* param;
}Task;

/**
 * prints error in sys call to stderr.
 */
void printErrorInSysCallToSTDERR() {
    char error_msg[] = "Error in system call\n";
    write(STDERR_FD, error_msg, sizeof(error_msg));
}

/**
 * threads function. tasks are taken and executed by the threads in the thread pool from the tasks queue.
 * @param args expected ThreadPool*
 * @return void
 */
void* execute(void* args) {
    ThreadPool* tp = (ThreadPool*)args;
    struct os_queue* taskQueue = tp->tasksQueue;
    printf("New thread was created\n");

    while (!tp->stopped && !(tp->canInsert == 0 && osIsQueueEmpty(taskQueue))) {
        /* Lock must be taken to wait on conditional variable */
        pthread_mutex_lock(&(tp->queueLock));

        /* Wait on condition variable, check for spurious wakeups.
           When returning from pthread_cond_wait(), we own the lock. */
        if((osIsQueueEmpty(taskQueue)) && (!tp->stopped)) {
            printf("Busy\n");
            pthread_cond_wait(&(tp->notify), &(tp->queueLock));
        }
        pthread_mutex_unlock(&(tp->queueLock));

        pthread_mutex_lock(&(tp->lock));
        if (!(osIsQueueEmpty(taskQueue))) {
            // take task from the queue
            Task* task = osDequeue(taskQueue);
            pthread_mutex_unlock(&(tp->lock));
            // execute task
            task->computeFunc(task->param);
            free(task);
        }
        else {
            pthread_mutex_unlock(&(tp->lock));
        }
    }
}

/**
 * creates a thread pool struct.
 * @param numOfThreads number of threads in the thread pool.
 * @return reference to new thread pool struct if succeeded, NULL if failed.
 */
ThreadPool* tpCreate(int numOfThreads) {
/*    int out = open("output",  O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (out == -1) {
        printf("Failed to open output file\n");
        printErrorInSysCallToSTDERR();
        exit(SYS_CALL_FAILURE);
    }
    // replace standard output with output file
    if (dup2(out, STDOUT_FILENO) == -1) {
        printf("Failed to operate dup2 for out\n");
        printErrorInSysCallToSTDERR();
        exit(SYS_CALL_FAILURE);
    }*/

    ThreadPool* tp = (ThreadPool*)malloc(sizeof(ThreadPool));
    if (tp == NULL) {
        printf("Failure: allocate memory for thread pool struct");
        return NULL;
    }
    tp->numOfThreads = numOfThreads;

    tp->threads = (pthread_t*)malloc(sizeof(pthread_t) * tp->numOfThreads);
    if (tp->threads == NULL) {
        printf("Failure: allocate memory for threads array");
        return NULL;
    }

    tp->tasksQueue = osCreateQueue();
    pthread_mutex_init(&(tp->lock), NULL);
    tp->stopped = 0;
    tp->canInsert = 1;

    if (pthread_mutex_init(&(tp->queueLock), NULL) != 0 ||
            pthread_mutex_init(&(tp->queueLock), NULL) != 0 ||
            pthread_cond_init(&(tp->notify), NULL) != 0) {
        printf("Failure: initialize one required mutex or more\n");
        tpDestroy(tp, 0);
        return NULL;
    }

    int i;
    for (i = 0; i < tp->numOfThreads; i++) {
         if(pthread_create(&(tp->threads[i]), NULL, execute, (void *)tp) != 0) {
             printf("Failure: creating a thread failed.\n");
         }
    }

    return tp;
}

/**
 * inserts a task to the tasks queue of the thread pool.
 * @param threadPool thread pool
 * @param computeFunc task
 * @param param argument to the task
 * @return 0- success , -1 - fail
 */
int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param) {
    if(threadPool == NULL || computeFunc == NULL) {
        return FAILURE;
    }

    if (!(threadPool->canInsert)) {
        return FAILURE;
    }

    Task* task = (Task*)malloc(sizeof(Task));
    if (task == NULL) {
        printf("Failure: allocate memory for threads array");
        return FAILURE;
    }

    task->computeFunc = computeFunc;
    task->param = param;

    osEnqueue(threadPool->tasksQueue, (void *)task);

    pthread_mutex_lock(&(threadPool->queueLock));
    // wake up thread that wait as long as the tasks queue is empty
    if(pthread_cond_signal(&(threadPool->notify)) != 0) {
        printf("Failure: signal opertion in tpInsertTask\n");
    }
    pthread_mutex_unlock(&(threadPool->queueLock));
    return SUCCESS;
}

/**
 * Destroys the thread pool.
 * @param threadPool thread pool
 * @param shouldWaitForTasks 0 - dont wait for tasks in the queue, else - wait for tasks.
 */
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
    if (threadPool == NULL) {
        return;
    }

    pthread_mutex_lock(&destryLock);
    // first time enter to tpDestory with valid thread pool
    if ( threadPool->canInsert != 0) {
        threadPool->canInsert = 0;
        // make sure tpDestroy will ne called only once for thr thread pool
    } else {
        return;
    }
    pthread_mutex_unlock(&destryLock);


    if (shouldWaitForTasks == DONT_WAIT_FOR_TASKS) {
        threadPool->stopped = 1;
    }
    int i, err;

    pthread_mutex_lock(&(threadPool->queueLock));

    /* Wake up all worker threads */
    if((pthread_cond_broadcast(&(threadPool->notify)) != 0) ||
       (pthread_mutex_unlock(&(threadPool->queueLock)) != 0)) {
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

    //free memory
    while (!osIsQueueEmpty(threadPool->tasksQueue)) {
        printf("Task was erased from tasks queue\n");
        Task* task = osDequeue(threadPool->tasksQueue);
        free(task);
    }

    osDestroyQueue(threadPool->tasksQueue);
    free(threadPool->threads);
    pthread_mutex_destroy(&(threadPool->lock));
    pthread_mutex_destroy(&(threadPool->queueLock));
    pthread_mutex_destroy(&destryLock);
    free(threadPool);
}