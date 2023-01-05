/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */


#ifndef CHANNEL_H_QWERTY
#define CHANNEL_H_QWERTY

#include <semaphore.h>
#include <pthread.h>

typedef struct cell cell_t;

typedef struct channel channel_t;
struct channel {
    pthread_mutex_t mutex;
    sem_t   lock;
    sem_t   sem;
    cell_t* head;
    cell_t* tail;
    int     size;
};

void channel_init(channel_t* channel);
int channel_write(channel_t* channel, int elem);
int channel_read(channel_t* channel, int* elem);
int channel_timedread(channel_t* channel, int* item, struct timespec* ts);
void channel_destroy(channel_t* channel);


#endif
