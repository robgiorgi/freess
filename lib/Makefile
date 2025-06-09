#!/bin/make -f
#
# Unified Makefile for native (gcc) and wasm (emcc) builds

DEFIN    = -DSTDC_HEADERS=1 -DHAVE_UNISTD_H=1 -DDIRENT=1 
OBJS     = sui.o getopt.o getopt1.o queue.o
ARFLAGS  = r
AR       = ar
LIBTGT   = libsui.a

# Native compiler
CC_NATIVE = gcc
CFLAGS_NATIVE = -O3 -D_Linux_ $(DEFIN) -I../lib -I./include

# WebAssembly compiler
CC_WASM   = emcc
CFLAGS_WASM = -O3 -D_Linux_ $(DEFIN) -I../lib -I./include

.PHONY: all clean wasm

all: clean native

native: CC=$(CC_NATIVE)
native: CFLAGS=$(CFLAGS_NATIVE)
native: $(LIBTGT)

wasm: CC=$(CC_WASM)
wasm: CFLAGS=$(CFLAGS_WASM)
wasm: $(LIBTGT)

$(LIBTGT): $(OBJS)
	$(AR) $(ARFLAGS) $(LIBTGT) $(OBJS)

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -c $<

sui.o: sui.c sui.h
getopt.o: getopt.c getopt.h
getopt1.o: getopt1.c getopt1.h
queue.o: queue.c sui.h

clean:
	-rm -f *.o *.ln $(LIBTGT)
