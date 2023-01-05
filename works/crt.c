/*
 *
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 *
 */


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <assert.h>

typedef struct crt crt_t;

typedef int (*crt_function_t)(crt_t *crt);


struct crt {
    crt_function_t     function;           // Actual co-routine function
    ucontext_t         suspend_context;    // Stores context previous to crtutine jump
    ucontext_t         resume_context;     // Stores coroutine context
    int                yield_value;        // Coroutine return/yield value
    bool               is_finished;        // To indicate the current coroutine status
};

void crt_yield(crt_t *crt, int value);
int crt_resume(crt_t *crt);

static void _crt_entry_point(crt_t *crt) {
    int return_value = crt->function(crt);
    crt->is_finished = true;
    crt_yield(crt, return_value);
}

crt_t *new_crt(crt_function_t function) {
    crt_t *crt = calloc(1, sizeof(*crt));

    crt->is_finished = false;
    crt->function = function;
    crt->resume_context.uc_stack.ss_sp = malloc(MINSIGSTKSZ);
    crt->resume_context.uc_stack.ss_size = MINSIGSTKSZ;
    crt->resume_context.uc_link = 0;

    getcontext(&crt->resume_context);
    makecontext(&crt->resume_context, (void (*)())_crt_entry_point, 1, crt);
    return crt;
}

int crt_resume(crt_t *crt) {
    if (crt->is_finished) return -1;
    swapcontext(&crt->suspend_context, &crt->resume_context);
    return crt->yield_value;
}

void crt_yield(crt_t *crt, int value) {
    crt->yield_value = value;
    swapcontext(&crt->resume_context, &crt->suspend_context);
}

void crt_free(crt_t *crt) {
    free(crt->resume_context.uc_stack.ss_sp);
    free(crt);
}

int hello_world(crt_t *crt) {
    puts("Hello");
    crt_yield(crt, 1);
    puts("World");
    return 2;
}

int main(int argc, char **argv) {
    crt_t *crt = new_crt(hello_world);

    assert(crt_resume(crt) == 1);     // Verifying return value
    assert(crt_resume(crt) == 2);     // Verifying return value
    assert(crt_resume(crt) == -1);    // Invalid call

    crt_free(crt);
    return 0;
}
