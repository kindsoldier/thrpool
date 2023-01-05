#
#  Copyright 2023 Oleg Borodin  <borodin@unix7.org>
#

all: test

CC = cc
CFLAGS = -O2 -Wall -I. -std=c99 -pthread
LDFLAGS = -pthread

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

syncer.c: syncer.h
syncer.o: syncer.c

channel.c: channel.h
channel.o: channel.c

worker.c: worker.h syncer.h
worker.o: worker.c

tools.c: tools.h
tools.o: tools.c

complex_test.c: syncer.h channel.h worker.h
complex_test.o: complex_test.c

channel_test.c: channel.h
channel_test.o: channel_test.c

OBJS += tools.o
OBJS += worker.o
OBJS += channel.o
OBJS += syncer.o

complex_test: complex_test.o $(OBJS)
	$(CC) $(LDFLAGS) -o $@ complex_test.o $(OBJS)

channel_test: channel_test.o $(OBJS)
	$(CC) $(LDFLAGS) -o $@ channel_test.o $(OBJS)


test: channel_test complex_test
	./channel_test
	./complex_test

clean:
	rm -f *_test
	rm -f *.o *~

#EOF
