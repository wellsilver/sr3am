cc = gcc

ifeq ($(OS),Windows_NT)
lib = -l gdi32
else
lib = -l X11
endif

all: lib examples

lib:
	gcc src/sr3am.c -c -o sr3am.o -g -Ofast -std=gnu2x
	ar rcs libsr3am.a sr3am.o

# This will randomly delete files and shit? wtf?
# %.o: examples/*.c
#	gcc -o $<.o $< -I include sr3am $(lib) -g

.PHONY: examples
examples:
	gcc examples/blank.c -o blank.o sr3am.o -I include $(lib) -g
	gcc examples/pong.c -o pong.o sr3am.o -I include $(lib) -g
	glslangValidator examples/compute.comp.glsl -V -o compute.spv.o
	gcc examples/compute.c -o compute.o sr3am.o -I include $(lib) -l vulkan -g -O0