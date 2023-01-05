/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */

#ifndef CHANNEL_H_QWERTY
#define CHANNEL_H_QWERTY

#include <semaphore.h>
#include <pthread.h>


typedef struct {
    pthread_mutex_t mutex;
    sem_t sem;
    int* items;
    int front;
    int rear;
    int size;
    int capa;
} channel_t;

channel_t* channel_new(int capa);
void channel_init(channel_t* channel, int capa);
int channel_size(channel_t* channel);
int channel_capa(channel_t* channel);
int channel_write(channel_t* channel, int x);
int channel_read(channel_t* channel, int* item);
int channel_timedread(channel_t* channel, int* item, struct timespec* ts);
void channel_destroy(channel_t* channel);
void channel_free(channel_t* channel);

#endif
