/*
 *
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <ucontext.h>

void assign(uint32_t *var, uint32_t val) {
    printf("hello #3\n");
    *var = val;
}

int main(int argc, char **argv) {

    uint32_t var = 0;
    ucontext_t ctx = { 0 };
    ucontext_t back = { 0 };

    getcontext(&ctx);

    ctx.uc_stack.ss_sp = calloc(1, MINSIGSTKSZ);
    ctx.uc_stack.ss_size = MINSIGSTKSZ;
    ctx.uc_stack.ss_flags = 0;

    ctx.uc_link = &back; 
    // ctx.uc_link = 0;

    makecontext(&ctx, (void (*)())assign, 2, &var, 100);
    swapcontext(&back, &ctx);

    printf("var = %d\n", var);

    return EXIT_SUCCESS;
}
