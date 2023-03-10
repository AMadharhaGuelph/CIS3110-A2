CC = gcc
CFLAGS = -Wall -g -std=c11

all: A2

A2: A2.o
	$(CC) A2.o -o A2
A2.o: A2.c header.h
	$(CC) $(CFLAGS) -c A2.c -o A2.o
clean:
	rm -f *.o *.hist A2