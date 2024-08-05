cc = gcc
exampledir = examples

ifeq ($(OS),Windows_NT)
lib = -l gdi+
else
lib = -l X11
endif

all: lib examples

lib:
	gcc src/sr3am.c -c -o sr3am -g -O0

%.o: examples/*.c
	gcc -o $<.o $< -I include sr3am $(lib) -g