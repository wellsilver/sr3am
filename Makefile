cc = gcc
exampledir = examples

all: lib examples

SRC=$(wildcard examples/*.c)

lib:
	gcc src/sr3am.c -c -o sr3am -g

examples: $(SRC)
	gcc -o $^.o $^ -I include sr3am -l gdi32 -g