/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdatomic.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>

#include <syncer.h>
#include <channel.h>
#include <worker.h>
#include <tools.h>

worker_t* new_worker(channel_t* channel, syncer_t* syncer, int ident) {
    worker_t* worker = malloc(sizeof(worker_t));
    worker->ident   = ident;
    worker->channel = channel;
    worker->syncer  = syncer;
    worker->doexit = 0;
    return worker;
}

void worker_init(worker_t* worker, channel_t* channel, syncer_t* syncer, int ident) {
    worker->ident   = ident;
    worker->channel = channel;
    worker->syncer  = syncer;
    worker->doexit = 0;
}

static void worker_doexit(worker_t* worker) {
    if (worker->doexit > 0) {
        printf("worker %d is canceled\n", worker->ident);
        syncer_done(worker->syncer);
        pthread_exit(NULL);
    }
}

static void* worker_start(void* argp) {
    worker_t* worker = (worker_t*)argp;
    printf("worker %d start\n", worker->ident);

    while(true) {
        struct timespec ts;
        ts_addsec(&ts, 1);

        int req = -1;
        int res = channel_timedread(worker->channel, &req, &ts);
        if (res < 0) {
            worker_doexit(worker);
            continue;
        }
        printf("worker %d run req %d\n", worker->ident, req);
        worker_doexit(worker);
    }

    printf("worker %d done\n", worker->ident);
    syncer_done(worker->syncer);
    pthread_exit(NULL);
}

void worker_run(worker_t* worker) {
    pthread_create(&(worker->thread), NULL, worker_start, (void*)worker);
    pthread_detach(worker->thread);
}

void worker_cancel(worker_t* worker) {
    worker->doexit++;
}

void worker_destroy(worker_t* worker) {
    // NOP
}

void worker_free(worker_t* worker) {
    free(worker);
}
