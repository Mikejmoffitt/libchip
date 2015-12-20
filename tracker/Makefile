# libchip tracker programs
# Copyright (C) Michael Moffitt 2015

# C compiler configuration
CC := clang
CFLAGS := -std=c99 -O2 -g -Wall -Werror -Iinc -pedantic
SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:.c=.o)
LDFLAGS := -lchip `pkg-config --cflags --libs --static allegro-static-5 allegro_audio-static-5 allegro_dialog-static-5 allegro_main-static-5 allegro_primitives-static-5 allegro_image-static-5`
EXECUTABLE := chipper

.PHONY: all clean

all: $(EXECUTABLE)

clean:
	$(RM) $(OBJECTS) $(OBJECTS)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@
