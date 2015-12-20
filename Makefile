# libchip
# Copyright (C) Michael Moffitt 2015

# C compiler configuration
CC := clang
CFLAGS := -std=c99 -O2 -g -Wall
INCLUDE := -Iinc 
LDFLAGS := 
# Archiver for static building
AR := ar
ARFLAGS := cvq

all: chipkernel.o chipkernel.o libchip.a

debug: debugcfg libchip.o chipkernel.o libchip.o libchip.a

chipkernel.o: src/chipkernel.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LDFLAGS) -c src/chipkernel.c -o chipkernel.o

libchip.o: src/libchip.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LDFLAGS) -c src/libchip.c -o libchip.o

libchip.a: libchip.o chipkernel.o 
	$(AR) $(ARFLAGS) libchip.a libchip.o chipkernel.o
	rm libchip.o
	rm chipkernel.o

.PHONY: install
install:
	cp libchip.a /usr/local/lib/
	chown root /usr/local/lib/libchip.a
	chmod 0775 /usr/local/lib/libchip.a
	cp inc/libchip.h /usr/local/include/
	chown root /usr/local/include/libchip.h
	chmod 0775 /usr/local/include/libchip.h

.PHONY: clean
clean:
	$(RM) chipkernel.o libchip.o libchip.a
