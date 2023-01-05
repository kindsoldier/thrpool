/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

typedef struct {
    sem_t sem;
    int ident;
    int state;
} hello_t;

hello_t* new_hello(int ident) {
    hello_t* hello = malloc(sizeof(hello_t));
    hello->ident = ident;
    hello->state = 0;
    sem_init(&(hello->sem), 1, 0);
    return hello;
}

void* hello_start(void* args) {
    hello_t* hello = (hello_t*)args;
    sleep(1);
    printf("hello %d started\n", hello->ident);
    hello->state = 2;
    sem_post(&(hello->sem));
    pthread_exit(hello);
}

void hello_run(hello_t* hello) {
    pthread_t thread;
    pthread_attr_t thattr;
    pthread_attr_init(&thattr);

    pthread_create(&thread, &thattr, hello_start, hello);
    pthread_attr_destroy(&thattr);

    pthread_detach(thread);

    //void* thres = NULL;
    //pthread_join(thread, (void**)&thres);

    //hello = (hello_t*)thres;
    //printf("hello %d done %d\n", hello->ident, hello->state);
    return;
}

void hello_wait(hello_t* hello) {
    sem_wait(&(hello->sem));
}

void hello_free(hello_t* hello) {
    free(hello);
}

int main(int argc, char **argv) {

    printf("main started\n");

    hello_t* hello1 = new_hello(1);
    hello_run(hello1);

    hello_t* hello2 = new_hello(2);
    hello_run(hello2);

    hello_wait(hello1);
    hello_wait(hello2);

    printf("hello %d done %d\n", hello1->ident, hello1->state);
    printf("hello %d done %d\n", hello2->ident, hello2->state);

    hello_free(hello1);
    hello_free(hello2);

    return 0;
}
