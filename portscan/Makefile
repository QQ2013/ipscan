CC=gcc
CFLAGS=-Wall -g -DDEBUG
LDFLAGS= -g
LIBS=-lpthread

all:portscan testsem

portscan:portscan.c
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)
testsem:testsem.c
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

clean:
	rm -rf *.o
	rm -f portscan
	rm -f testsem
