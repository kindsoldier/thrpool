/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <assert.h>
#include <time.h>


typedef struct itask task_t;

typedef void (*task_f)(task_t* task);

struct itask {
    int             ident;
    task_f          func;
    ucontext_t*     scntx;
    ucontext_t*     tcntx;
    task_t*         next;
    bool            done;
};


#define STACKSIZE SIGSTKSZ

void task_start(task_t* task);
void task_resume(task_t* task);
void task_yield(task_t* task);

task_t* new_task(int ident, ucontext_t* scntx, task_f func) {
    task_t* task = malloc(sizeof(task_t));
    task->scntx = scntx;
    task->func = func;
    task->done = false;
    task->next = NULL;
    task->ident = ident;

    ucontext_t* tcntx = malloc(sizeof(ucontext_t));
    getcontext(tcntx);
    tcntx->uc_stack.ss_sp = malloc(STACKSIZE);
    tcntx->uc_stack.ss_size = STACKSIZE;
    tcntx->uc_link = NULL;
    makecontext(tcntx, (void (*)())task_start, 1, task);
    task->tcntx = tcntx;

    return task;
}

void task_resume(task_t* task) {
    swapcontext(task->scntx, task->tcntx);
}

void task_start(task_t* task) {
    if (task->done == true) { return; }
    task->func(task);
    printf("task %d done\n", task->ident);
    task->done = true;
    swapcontext(task->tcntx, task->scntx);
}

void task_yield(task_t* task) {
    swapcontext(task->tcntx, task->scntx);
}

void task_free(task_t* task) {
    free(task->tcntx);
    free(task);
}

typedef struct ischeduler scheduler_t;
struct ischeduler{
    task_t*         head;
    int             size;
    ucontext_t*     mcntx;
    ucontext_t*     cntx;
};

void scheduler_start(scheduler_t* scheduler);

scheduler_t* new_scheduler() {
    scheduler_t* scheduler = malloc(sizeof(scheduler_t));
    scheduler->size = 0;
    scheduler->head = NULL;

    ucontext_t* cntx = malloc(sizeof(ucontext_t));
    getcontext(cntx);
    cntx->uc_stack.ss_sp = malloc(STACKSIZE);
    cntx->uc_stack.ss_size = STACKSIZE;
    cntx->uc_link = NULL;
    makecontext(cntx, (void (*)())scheduler_start, 1, scheduler);

    scheduler->cntx = cntx;

    return scheduler;
}

task_t* scheduler_getn(scheduler_t* scheduler) {
    int i = 0;
    int num = rand() % scheduler->size;
    task_t* curr = scheduler->head;
    while (curr != NULL) {
        if (i == num) return curr;
        curr = curr->next;
        i++;
    };
    return NULL;
}

void scheduler_clean(scheduler_t* scheduler) {
    task_t* curr = scheduler->head;
    while (curr->next != NULL) {
        if (curr->next->done) {
            task_t* done = curr->next;
            curr->next = curr->next->next;
            task_free(done);
            scheduler->size--;
            continue;
        }
        curr = curr->next;
    };
    if (scheduler->head != NULL) {
        if (scheduler->head->done) {
            task_t* done = scheduler->head;
            scheduler->head = scheduler->head->next;
            task_free(done);
            scheduler->size--;
        }
    }
    return;
}

void scheduler_run(scheduler_t* scheduler) {
    ucontext_t* mcntx = malloc(sizeof(ucontext_t));
    getcontext(mcntx);
    scheduler->mcntx = mcntx;
    swapcontext(scheduler->mcntx, scheduler->cntx);
 }

void scheduler_start(scheduler_t* scheduler) {
    printf("scheduler start\n");
    srand(time(NULL));
    while (scheduler->size > 0) {
        task_t* todo = scheduler_getn(scheduler);
        task_resume(todo);
        scheduler_clean(scheduler);
    }
    printf("scheduler end\n");
    swapcontext(scheduler->cntx, scheduler->mcntx);
    return;
}

void scheduler_add(scheduler_t* scheduler, task_t* new) {
    if (scheduler->head == NULL) {
        scheduler->head = new;
        scheduler->size++;
        return;
    }
    task_t* curr = scheduler->head;
    while (curr->next != NULL) {
        curr = curr->next;
    };
    curr->next = new;
    scheduler->size++;
    return;
}

void scheduler_free(scheduler_t* scheduler) {
    free(scheduler->cntx);
    free(scheduler->mcntx);
    free(scheduler);
}

void task(task_t* task) {
    printf("task %d #1\n", task->ident);
    task_yield(task);
    printf("task %d #2\n", task->ident);
    task_yield(task);
    printf("task %d #3\n", task->ident);
    return;
}

int main(int argc, char **argv) {

    printf("main start\n");
    scheduler_t* scheduler = new_scheduler();

    scheduler_add(scheduler, new_task(1, scheduler->cntx, task));
    scheduler_add(scheduler, new_task(2, scheduler->cntx, task));
    scheduler_add(scheduler, new_task(3, scheduler->cntx, task));

    scheduler_run(scheduler);
    scheduler_free(scheduler);

    printf("main end\n");
    return 0;
}
