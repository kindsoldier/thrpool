/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */

#include <stdlib.h>

#include <syncer.h>

syncer_t* new_syncer() {
    syncer_t* syncer = malloc(sizeof(syncer_t));
    syncer->num = 0;
    sem_init(&(syncer->sem), 1, 0);
    return syncer;
}

void syncer_add(syncer_t* syncer) {
    syncer->num++;
}

void syncer_done(syncer_t* syncer) {
    if ((--syncer->num) == 0) sem_post(&(syncer->sem));
}

void syncer_wait(syncer_t* syncer) {
    if ((syncer->num) < 1) sem_post(&(syncer->sem));
    sem_wait(&(syncer->sem));
}

void syncer_init(syncer_t* syncer) {
    syncer->num = 0;
    sem_init(&(syncer->sem), 1, 0);
}

void syncer_destroy(syncer_t* syncer) {
    sem_destroy(&(syncer->sem));
}

void syncer_free(syncer_t* syncer) {
    sem_destroy(&(syncer->sem));
    free(syncer);
}
