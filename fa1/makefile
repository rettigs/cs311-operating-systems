all: prog

sieve.o: sieve.c sieve.h
        gcc -g -Wall -std=c99 -c sieve.c

prog: sieve.o
        gcc -g -Wall -std=c99 -o prog sieve.o
