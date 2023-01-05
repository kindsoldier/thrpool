/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <semaphore.h>

#include <channel.h>

struct cell {
    cell_t* next;
    int     data;
};


static cell_t* new_cell(int data) {
    cell_t* cell = malloc(sizeof(cell_t));
    cell->data = data;
    cell->next = NULL;
    return cell;
}

static void cell_free(cell_t* cell) {
    free(cell);
}

void channel_init(channel_t* channel) {
    sem_init(&(channel->lock), 0, 1);
    sem_init(&(channel->sem), 0, 0);

    channel->head = NULL;
    channel->tail = NULL;
    channel->size = 0;
    return;
}

void channel_destroy(channel_t* channel) {
    sem_wait(&(channel->lock));
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
    sem_post(&(channel->lock));

    sem_destroy(&(channel->sem));
    sem_destroy(&(channel->lock));
}

int channel_write(channel_t* channel, int elem) {
    sem_wait(&(channel->lock));
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
    sem_post(&(channel->lock));
    sem_post(&(channel->sem));
    return res;
}

int channel_read(channel_t* channel, int* elem) {
    sem_wait(&(channel->sem));
    sem_wait(&(channel->lock));
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
    sem_post(&(channel->lock));
    return res;
}

int channel_timedread(channel_t* channel, int* elem, struct timespec* ts) {
    sem_timedwait(&(channel->sem), ts);
    sem_wait(&(channel->lock));
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
    sem_post(&(channel->lock));
    return res;
}
