CC=gcc
CFLAGS=-Wall -g -std=gnu11
LDFLAGS=
CLIBS=

all: myshell
myshell: myshell.o main.o

myshell.o: myshell.c myshell.h
main.o: main.c myshell.h

clean:
	-rm -rf myshell myshell.o main.o
tidy: clean
	-rm -rf *~

.PHONY: all clean
