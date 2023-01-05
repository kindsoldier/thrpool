/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>


typedef struct {
    int         ident;
    char*       message;
    sem_t       execsem;
    sem_t       exitsem;
    sem_t       donesem;
    atomic_int  doexit;
    pthread_t   thread;
} worker_t;

void* worker_handler(void* argp);

worker_t* new_worker(int ident, char* message) {
    worker_t* worker = (worker_t*)malloc(sizeof(worker_t));
    worker->ident = ident;
    worker->message = malloc(strlen(message) + 1);
    worker->doexit = 0;

    strcpy(worker->message, message);

    sem_init(&(worker->execsem), 1, 0);
    sem_init(&(worker->donesem), 1, 0);

    pthread_create(&(worker->thread), NULL, worker_handler, (void*)worker);
    pthread_detach(worker->thread);
    return worker;
}

void worker_notify(worker_t* worker) {
    sem_post(&(worker->execsem));
}

void* worker_handler(void* argp) {
    worker_t* worker = (worker_t*)argp;

    while(true) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;

        int semres = sem_timedwait(&(worker->execsem), &ts);
        if (semres < 0) {
            if (worker->doexit > 0) {
                printf("worker %d is canceled\n", worker->ident);
                sem_post(&(worker->donesem));
                pthread_exit(NULL);
            }
            continue;
        }
        printf("worker %d message: %s", worker->ident, worker->message);
    }
    pthread_exit(NULL);
}

void worker_cancel(worker_t* worker) {
    worker->doexit++;
}

void worker_wait(worker_t* worker) {
    sem_wait(&(worker->donesem));
}

void worker_free(worker_t* worker) {
    int ident = worker->ident;

    sem_destroy(&(worker->exitsem));
    sem_destroy(&(worker->donesem));
    sem_destroy(&(worker->execsem));

    free(worker->message);
    free(worker);
    printf("worker %d is free\n", ident);
}

void signal_handler(int signum) {
    printf("thread signal %d\n", signum);
}

int main(int argc, char **argv) {

    printf("main started\n");
    signal(SIGUSR1,signal_handler);

    worker_t* worker = new_worker(1, "hello\n");

    int count = 5;
    for (int i = 0; i < count; i++) {
        worker_notify(worker);
    }

    worker_cancel(worker);
    worker_wait(worker);
    worker_free(worker);
    printf("main done\n");

    return 0;
}
