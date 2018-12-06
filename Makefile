CC = gcc

all:
	$(CC) -std=c99 -c -I /opt/AMDAPP/include mandelocl.c -o mandelocl.o `sdl-config --cflags --libs`
	$(CC) mandelocl.o -o mandelocl -L /opt/AMDAPP/lib/x86_64/ -l OpenCL `sdl-config --cflags --libs`
	rm *.o