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

typedef struct queue {
    pthread_mutex_t mutex;
    sem_t sem;
    int* items;
    int front;
    int rear;
    int size;
    int capa;

} channel_t;

channel_t* channel_new(int capa) {
    channel_t* channel = (channel_t*)malloc(sizeof(channel_t));
    if (channel == NULL) return NULL;

    if (pthread_mutex_init(&(channel->mutex), NULL) < 0) {
        free(channel);
        return NULL;
    }

    if (sem_init(&(channel->sem), 1, 0) < 0) {
        free(channel);
        return NULL;
    }

    channel->items = (int*)malloc(capa * sizeof(int));
    memset(channel->items, 0, capa * sizeof(int));
    if (channel->items == NULL) {
        pthread_mutex_destroy(&(channel->mutex));
        free(channel);
        return NULL;
    }
    channel->capa = capa;
    channel->front = -1;
    channel->rear = 0;
    channel->size = 0;
    return channel;
}

int channel_size(channel_t* channel) {
    if (channel == NULL) return -1;
    pthread_mutex_lock(&(channel->mutex));
    int res = channel->size;
    pthread_mutex_unlock(&(channel->mutex));
    return res;
}

int channel_capa(channel_t* channel) {
    if (channel == NULL) return -1;
    pthread_mutex_lock(&(channel->mutex));
    int res = channel->capa;
    pthread_mutex_unlock(&(channel->mutex));
    return res;
}

int channel_write(channel_t* channel, int x) {
    if (channel == NULL) return -1;
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
    if (channel == NULL) return res;
    if (item == NULL) return res;

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

void queue_free(channel_t* channel) {
    if (channel == NULL) return;
    pthread_mutex_destroy(&(channel->mutex));
    free(channel->items);
    free(channel);
}

int main(int argc, char **argv) {
    int capa = 12;
    int size = -1;
    channel_t* channel = channel_new(capa);
    size = channel_size(channel);
    capa = channel_capa(channel);
    printf("channel size = %d, capa = %d\n", size, capa);

    int res = -1;
    for (int i = 0; i < channel_capa(channel); i++) {
        res = channel_write(channel, i);
        size = channel_size(channel);
        capa = channel_capa(channel);
        printf("channel write: size = %d, capa = %d, res = %d\n", size, capa, res);
    }

    int elem = -1;
    for (int i = 0; i < channel_capa(channel); i++) {
        res = channel_read(channel, &elem);
        size = channel_size(channel);
        capa = channel_capa(channel);
        printf("channel read: elem %d = %d, res = %d, size = %d\n", i, elem, res, size);
    }
    return 0;
}
