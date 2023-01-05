/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */

#include <unistd.h>
#include <time.h>

int msleep(uint tms) {
  return usleep(tms * 1000);
}

void ts_addsec(struct timespec* ts, int sec) {
    clock_gettime(CLOCK_REALTIME, ts);
    ts->tv_sec += (time_t)sec;
}

void ts_addmsec(struct timespec* ts, long msec) {
    clock_gettime(CLOCK_REALTIME, ts);
    ts->tv_nsec += (time_t)msec * 1000 * 1000;
}

void ts_addusec(struct timespec* ts, long usec) {
    clock_gettime(CLOCK_REALTIME, ts);
    ts->tv_nsec += (time_t)usec * 1000;
}

long getnanotime() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000 * 1000 * 1000 + ts.tv_nsec;
}
