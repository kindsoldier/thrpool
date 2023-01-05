/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <semaphore.h>
#include <pthread.h>

typedef struct cell cell_t;
struct cell {
    cell_t* next;
    int     data;
};

cell_t* new_cell(int data) {
    cell_t* cell = malloc(sizeof(cell_t));
    cell->data = data;
    cell->next = NULL;
    return cell;
}

void cell_free(cell_t* cell) {
    free(cell);
}

typedef struct channel channel_t;
struct channel {
    pthread_mutex_t mutex;
    sem_t   sem;
    cell_t* head;
    cell_t* tail;
    int     size;
};

void channel_init(channel_t* channel) {
    pthread_mutex_init(&(channel->mutex), 0);
    sem_init(&(channel->sem), 0, 0);

    channel->head = NULL;
    channel->tail = NULL;
    channel->size = 0;

    return;
}

void channel_destroy(channel_t* channel) {
    pthread_mutex_lock(&(channel->mutex));
    int qsize = channel->size;
    for (int i = 0; i < qsize; i++) {
        switch(channel->size) {
            case 0: {
                break;
            }
            case 1: {
                cell_free(channel->head);
                channel->head = NULL;
                channel->tail = NULL;
                channel->size = 0;
                break;
            }
            default: {
                cell_t* oldcell = channel->head;
                channel->head = channel->head->next;
                channel->size--;
                cell_free(oldcell);
                break;
            }
        }
    }
    pthread_mutex_unlock(&(channel->mutex));

    pthread_mutex_destroy(&(channel->mutex));
    sem_destroy(&(channel->sem));
}

int channel_write(channel_t* channel, int elem) {
    pthread_mutex_lock(&(channel->mutex));
    int res = 0;
    cell_t* newcell = new_cell(elem);
    uintptr_t headptr = (uintptr_t)channel->head;
    switch(headptr) {
        case (uintptr_t)NULL: {
            channel->head = newcell;
            channel->tail = newcell;
            channel->size++;
            res = 1;
            break;
        }
        default: {
            channel->tail->next = newcell;
            channel->tail = newcell;
            channel->size++;
            res = 1;
            break;
        }
    }
    sem_post(&(channel->sem));
    pthread_mutex_unlock(&(channel->mutex));
    return res;
}

int channel_read(channel_t* channel, int* elem) {
    sem_wait(&(channel->sem));
    pthread_mutex_lock(&(channel->mutex));
    int res = 0;
    switch(channel->size) {
        case 0: {
            res = -1;
            break;
        }
        case 1: {
            *elem = channel->head->data;
            cell_free(channel->head);
            channel->head = NULL;
            channel->tail = NULL;
            channel->size = 0;
            res = 1;
            break;
        }
        default: {
            *elem = channel->head->data;
            cell_t* oldcell = channel->head;
            channel->head = channel->head->next;
            channel->size--;
            cell_free(oldcell);
            res = 1;
            break;
        }
    }
    pthread_mutex_unlock(&(channel->mutex));
    return res;
}

int main(int argc, char **argv) {

    channel_t channel;
    channel_init(&channel);

    int count = 12;
    for (int i = 0; i < count; i++) {
        channel_write(&channel, i);
    }
    printf("channel size = %d\n", channel.size);

    int qsize = channel.size;
    for (int i = 0; i < qsize; i++) {
        int elem = -1;
        int res = channel_read(&channel, &elem);
        printf("channel elem = %d, size = %d, res = %d\n", elem, channel.size, res);
    }

    channel_destroy(&channel);

    return 0;
}
