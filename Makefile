cc = gcc

ifeq ($(OS),Windows_NT)
lib = -l gdi32
else
lib = -l X11
endif

all: lib examples

lib:
	gcc src/sr3am.c -c -o sr3am.o -g -O0

# This will randomly delete files and shit? wtf?
# %.o: examples/*.c
#	gcc -o $<.o $< -I include sr3am $(lib) -g

examples:
	gcc examples/blank.c -o blank.o sr3am.o -I include $(lib)
	gcc examples/pong.c -o pong.o sr3am.o -I include $(lib)