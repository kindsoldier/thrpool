/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */


#ifndef WORKER_H_QWERTY
#define WORKER_H_QWERTY

#include <stdatomic.h>
#include <semaphore.h>
#include <pthread.h>

#include <syncer.h>
#include <channel.h>

typedef struct {
    int         ident;
    channel_t*  channel;
    syncer_t*   syncer;
    pthread_t   thread;
    atomic_int  doexit;
} worker_t;


worker_t* new_worker(channel_t* channel, syncer_t* syncer, int ident);
void worker_init(worker_t* worker, channel_t* channel, syncer_t* syncer, int ident);

void worker_run(worker_t* worker);
void worker_cancel(worker_t* worker);
void worker_destroy(worker_t* worker);
void worker_free(worker_t* worker);

#endif
