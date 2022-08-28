CFLAGS=-Wall -Wpedantic $(shell sdl2-config --cflags) -g
LIBRARIES := $(shell sdl2-config --libs) -lm -lcglm
UNAME := $(shell uname -s)

ifeq ($(UNAME), Darwin)
	CFLAGS += -I/opt/homebrew/include
	LIBRARIES += -framework OpenGL -L/opt/homebrew/lib
else ifeq ($(UNAME), Linux)
	LIBRARIES += -lGL
endif

.DEFAULT_GOAL := all

christmas_tree.o: christmas_tree.c shader.c spiral.c
	gcc $(CFLAGS) -c -o $@ $<

christmas_tree: christmas_tree.o shader.o spiral.o
	gcc -o $@ $^ $(LIBRARIES)

all: christmas_tree

clean:
	rm -f *.o christmas_tree || true