/*
 * Copyright 2022 Oleg Borodin  <borodin@unix7.org>
 */

#ifndef TOOLS_H_QWERTY
#define TOOLS_H_QWERTY

int msleep(uint tms);

void ts_addsec(struct timespec* ts, int sec);
void ts_addmsec(struct timespec* ts, long msec);
void ts_addusec(struct timespec* ts, long usec);

long getnanotime();

#endif
