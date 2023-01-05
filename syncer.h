/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */

#ifndef SYNCER_H_QWERTY
#define SYNCER_H_QWERTY

#include <semaphore.h>
#include <stdatomic.h>

typedef struct {
    sem_t       sem;
    atomic_int  num;
} syncer_t;

syncer_t* new_syncer();
void syncer_add(syncer_t* syncer);
void syncer_done(syncer_t* syncer);
void syncer_wait(syncer_t* syncer);
void syncer_init(syncer_t* syncer);
void syncer_destroy(syncer_t* syncer);
void syncer_free(syncer_t* syncer);


#endif
