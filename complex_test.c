/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <channel.h>
#include <syncer.h>
#include <worker.h>

int main(int argc, char **argv) {

    srand(1);
    printf("main started\n");

    syncer_t syncer;
    syncer_init(&syncer);

    channel_t channel;
    channel_init(&channel);

    int wcount = 4;
    worker_t workers[wcount];
    for (int i = 0; i < wcount; i++) {
        worker_t* worker = &(workers[i]);
        int ident = i + 1;
        worker_init(worker, &channel, &syncer, ident);
        syncer_add(&syncer);
        worker_run(worker);
    }

    sleep(1);

    int jobs = 12;
    for (int i = 0; i < jobs; i++) {
        channel_write(&channel, i);
    }

    sleep(1);

    for (int i = 0; i < wcount; i++) {
        worker_cancel(&(workers[i]));
    }
    syncer_wait(&syncer);

    for (int i = 0; i < wcount; i++) {
        worker_destroy(&(workers[i]));
    }

    syncer_destroy(&syncer);
    channel_destroy(&channel);

    printf("main done\n");
    return 0;
}
