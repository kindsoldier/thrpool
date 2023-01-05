/*
 * Copyright 2023 Oleg Borodin  <borodin@unix7.org>
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <channel.h>
#include <syncer.h>
#include <worker.h>
#include <tools.h>

void test01(void) {
    long alltime = 0;
    int rcount = 100;

    for (int i = 0; i < rcount; i++) {
        channel_t channel;
        channel_init(&channel);

        long start = getnanotime();
        int ecount = 1000 * 1000;
        for (int i = 0; i < ecount; i++) {
            channel_write(&channel, i);
        }
        for (int i = 0; i < ecount; i++) {
            int elem;
            channel_read(&channel, &elem);
        }
        long stop = getnanotime();
        alltime += (stop - start) / ecount;

        channel_destroy(&channel);
    }
    printf("time per rw: %ld ns\n", alltime / rcount);
}

int main(int argc, char **argv) {
    test01();
    return 0;
}
