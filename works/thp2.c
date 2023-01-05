/*
 *
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 *
 */



#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>


typedef struct {
    int         ident;
    char*       message;
    sem_t       runsem;
    //sem_t       extsem;
    pthread_t   thread;
} worker_t;

void* worker_handler(void* argp);

worker_t* new_worker(int ident, char* message) {
    worker_t* worker = (worker_t*)malloc(sizeof(worker_t));
    worker->ident = ident;
    worker->message = malloc(strlen(message) + 1);
    strcpy(worker->message, message);
    sem_init(&(worker->runsem), 1, 0);
    //sem_init(&(worker->extsem), 1, 0);
    pthread_create(&(worker->thread), NULL, worker_handler, (void*)worker);
    pthread_detach(worker->thread);
    return worker;
}

void worker_notify(worker_t* worker) {
    sem_post(&(worker->runsem));
}

void* worker_handler(void* argp) {
    worker_t* worker = (worker_t*)argp;
    while(true) {
        sem_wait(&(worker->runsem));
        printf("worker %d message: %s", worker->ident, worker->message);
    }
    return NULL;
}

void worker_free(worker_t* worker) {
    sem_destroy(&(worker->runsem));
    free(worker->message);
    free(worker);
}

int main(int argc, char **argv) {
    worker_t* worker = new_worker(1, "hello\n");
    for (int i = 0; i < 10; i++) {
        worker_notify(worker);
    }
    sleep(1);
    return 0;
}
