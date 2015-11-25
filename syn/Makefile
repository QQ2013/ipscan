CC=gcc
CFLAGS=-Wall -g -DDEBUG
LDFLAGS= -g
LIBS=-lpthread

all:synscan

synscan:synscan.c
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

clean:
	rm -rf *.o
	rm -f synscan
