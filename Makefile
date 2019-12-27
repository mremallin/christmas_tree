CFLAGS=-Wall -Wpedantic $(shell sdl2-config --cflags)
LIBRARIES=$(shell sdl2-config --libs)

.DEFAULT_GOAL := all

christmas_tree.o: christmas_tree.c
	gcc $(CFLAGS) -c -o $@ $<

christmas_tree: christmas_tree.o
	gcc -o $@ $^ $(LIBRARIES)

all: christmas_tree

.PHONY: christmas_tree

clean:
	rm -f *.o christmas_tree || true