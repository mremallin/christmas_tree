CFLAGS=-Wall -Wpedantic $(shell sdl2-config --cflags) -g
LIBRARIES=$(shell sdl2-config --libs) -framework OpenGL

.DEFAULT_GOAL := all

christmas_tree.o: christmas_tree.c shader.c
	gcc $(CFLAGS) -c -o $@ $<

christmas_tree: christmas_tree.o shader.o
	gcc -o $@ $^ $(LIBRARIES)

all: christmas_tree

clean:
	rm -f *.o christmas_tree || true