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

examples:
	gcc examples/blank.c -o blank.o sr3am.o -I include $(lib) -g
	gcc examples/pong.c -o pong.o sr3am.o -I include $(lib) -g
ifeq ($(OS),Windows_NT)
	ld -r -o embeds.o -b binary examples/opencl.cl
else
	ld -r -o embeds.o -z noexecstack -b binary examples/opencl.cl
endif
	objcopy --rename-section .data=.rodata,alloc,load,readonly,data,contents embeds.o
	gcc examples/opencl.c -o opencl.o sr3am.o embeds.o -I include $(lib) -l OpenCL -g -Ofast