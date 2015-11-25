CC=gcc
CFLAGS=-Wall -g -DDEBUG
LDFLAGS= -g
LIBS=-lpthread

all:scanserver scanclient

scanserver:scanserver.c
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

scanclient:scanclient.c
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

clean:
	rm -rf *.o
	rm -f scanserver scanclient
