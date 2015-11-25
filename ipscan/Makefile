CC=gcc
CFLAGS=-Wall -g -DDEBUG
LDFLAGS=
LIBS=-lpthread

all:ping_ip.o pingsend pingrecv

ping_ip.o: ping_ip.c
	$(CC) -o $@ $(LDFLAGS) -c $^ $(LIBS)
pingsend: pingsend.c ping_ip.o 
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)
pingrecv: pingrecv.c ping_ip.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

clean:
	rm -rf *.o
	rm pingsend pingrecv -f
