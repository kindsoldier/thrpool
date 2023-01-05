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
#include <pthread.h>


typedef struct {
    pthread_t*      thread;
    pthread_cond_t  cond;
    pthread_mutex_t mutex;
    char*           message;
} worker_t;

worker_t* new_worker(void* handler(void*)) {
    worker_t* worker = (worker_t*)malloc(sizeof(worker_t));

    pthread_mutex_init(&(worker->mutex), NULL);
    pthread_cond_init(&(worker->cond), NULL);

    worker->thread = (pthread_t*)malloc(sizeof(pthread_t));
    pthread_create(worker->thread, NULL, handler, (void*)worker);
    pthread_detach(*(worker->thread));

    return worker;
}

void worker_notify(worker_t* worker) {
    pthread_mutex_unlock(&(worker->mutex));
    pthread_cond_broadcast(&(worker->cond));
}

void* handler(void* argp) {
    worker_t* worker = (worker_t*)argp;
    while(true) {
        pthread_mutex_lock(&(worker->mutex));
        pthread_cond_wait(&(worker->cond), &(worker->mutex));
        printf("%s", worker->message);
        pthread_mutex_unlock(&(worker->mutex));
    }
    return NULL;
}


int main(int argc, char **argv) {

    worker_t* worker = new_worker(handler);
    worker->message = "hello\n";

    sleep(1);

    for (int i = 0; i < 10; i++) {
        sleep(1);
        worker_notify(worker);
    }


    sleep(1);
    //while (true) {}

    return 0;
}
