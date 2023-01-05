/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */

#include <stdlib.h>
#include <semaphore.h>
#include <string.h>

#include <channel.h>

channel_t* channel_new(int capa) {
    channel_t* channel = (channel_t*)malloc(sizeof(channel_t));

    channel->items = (int*)malloc(capa * sizeof(int));
    memset(channel->items, 0, capa * sizeof(int));

    pthread_mutex_init(&(channel->mutex), NULL);
    sem_init(&(channel->sem), 1, 0);

    channel->capa = capa;
    channel->front = -1;
    channel->rear = 0;
    channel->size = 0;
    return channel;
}

void channel_init(channel_t* channel, int capa) {
    channel->items = (int*)malloc(capa * sizeof(int));
    memset(channel->items, 0, capa * sizeof(int));

    pthread_mutex_init(&(channel->mutex), NULL);
    sem_init(&(channel->sem), 1, 0);

    channel->capa = capa;
    channel->front = -1;
    channel->rear = 0;
    channel->size = 0;
    return;
}


int channel_size(channel_t* channel) {
    pthread_mutex_lock(&(channel->mutex));
    int res = channel->size;
    pthread_mutex_unlock(&(channel->mutex));
    return res;
}

int channel_capa(channel_t* channel) {
    pthread_mutex_lock(&(channel->mutex));
    int res = channel->capa;
    pthread_mutex_unlock(&(channel->mutex));
    return res;
}

int channel_write(channel_t* channel, int x) {
    int res = -1;
    pthread_mutex_lock(&(channel->mutex));
    if ((channel->size + 1) > channel->capa) {
        pthread_mutex_unlock(&(channel->mutex));
        return res;
    }
    channel->front = (channel->front + 1) % channel->capa;
    channel->items[channel->front] = x;
    channel->size++;
    res = 1;
    pthread_mutex_unlock(&(channel->mutex));
    sem_post(&(channel->sem));
    return res;
}

int channel_read(channel_t* channel, int* item) {
    int res = -1;
    sem_wait(&(channel->sem));
    pthread_mutex_lock(&(channel->mutex));
    if (channel->size == 0) {
        *item = 0;
        pthread_mutex_unlock(&(channel->mutex));
        return res;
    }
    *item = channel->items[channel->rear];
    channel->items[channel->rear] = 0;
    channel->rear = (channel->rear + 1) % channel->capa;
    channel->size--;
    res = 1;
    pthread_mutex_unlock(&(channel->mutex));
    return res;
}

int channel_timedread(channel_t* channel, int* item, struct timespec * ts) {
    int res = -1;
    res = sem_timedwait(&(channel->sem), ts);
    if (res < 0) {
        return res;
    }

    pthread_mutex_lock(&(channel->mutex));
    if (channel->size == 0) {
        *item = 0;
        pthread_mutex_unlock(&(channel->mutex));
        return res;
    }
    *item = channel->items[channel->rear];
    channel->items[channel->rear] = 0;
    channel->rear = (channel->rear + 1) % channel->capa;
    channel->size--;
    res = 1;
    pthread_mutex_unlock(&(channel->mutex));
    return res;
}

void channel_destroy(channel_t* channel) {
    pthread_mutex_destroy(&(channel->mutex));
    free(channel->items);
    channel->items = NULL;
}

void channel_free(channel_t* channel) {
    pthread_mutex_destroy(&(channel->mutex));
    free(channel->items);
    free(channel);
}
