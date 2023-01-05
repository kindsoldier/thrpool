/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */

#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>


typedef struct {
    sem_t       sem;
    atomic_int  num;
} wg_t;

wg_t* new_gw() {
    wg_t* wg = malloc(sizeof(wg_t));
    wg->num = 0;
    sem_init(&(wg->sem), 1, 0);
    return wg;
}

void wg_add(wg_t* wg) {
    wg->num++;
}

void wg_done(wg_t* wg) {
    if ((--wg->num) == 0) sem_post(&(wg->sem));
}

void wg_wait(wg_t* wg) {
    if ((wg->num) < 1) sem_post(&(wg->sem));
    sem_wait(&(wg->sem));
}

void wg_init(wg_t* wg) {
    wg->num = 0;
    sem_init(&(wg->sem), 1, 0);
}

void wg_destroy(wg_t* wg) {
    sem_destroy(&(wg->sem));
}

void wg_free(wg_t* wg) {
    sem_destroy(&(wg->sem));
    free(wg);
}

typedef struct {
    int         ident;
    int         loops;
    wg_t*       wg;
    pthread_t   thread;
} worker_t;


worker_t* new_worker(wg_t* wg, int ident, int loops) {
    worker_t* worker = malloc(sizeof(worker_t));
    worker->ident   = ident;
    worker->wg      = wg;
    worker->loops   = loops;
    return worker;
}

void worker_init(worker_t* worker, wg_t* wg, int ident, int loops) {
    worker->ident   = ident;
    worker->wg      = wg;
    worker->loops   = loops;
}

void* worker_start(void* argp) {
    worker_t* worker = (worker_t*)argp;
    printf("worker %d start\n", worker->ident);

    for (int i = 0; i < worker->loops; i++) {
        sleep(1);
    }
    printf("worker %d done\n", worker->ident);
    wg_done(worker->wg);
    pthread_exit(NULL);
}

void worker_run(worker_t* worker) {
    pthread_create(&(worker->thread), NULL, worker_start, (void*)worker);
    pthread_detach(worker->thread);
}

void worker_destroy(worker_t* worker) {
    // NOP
}

void worker_free(worker_t* worker) {
    free(worker);
}

#define WORKERS 10

int main(int argc, char **argv) {
    srand(1);
    printf("main started\n");

    wg_t wg;
    wg_init(&wg);

    worker_t workers[WORKERS];
    for (int i = 0; i < WORKERS; i++) {
        worker_t* worker = &(workers[i]);
        int loops = rand() % 5 + 1;
        int ident = i + 1;
        worker_init(worker, &wg, ident, loops);
        wg_add(&wg);
        worker_run(worker);
    }
    printf("wg num = %d\n", wg.num);
    wg_wait(&wg);
    wg_destroy(&wg);

    for (int i = 0; i < WORKERS; i++) {
        worker_destroy(&(workers[i]));
    }
    printf("wg num = %d\n", wg.num);
    printf("main done\n");
    return 0;
}
