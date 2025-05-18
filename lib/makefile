#!/bin/make -f
#
# AR:		archive command
# AS:		assembler command
# ASFLAGS:	options to assembler
# CC:		compiler command
# CFLAGS:	options to C compiler
#
#  Roberto Giorgi (giorgi@iet.unipi.it)
#  27/12/96

DEFIN	= -DSTDC_HEADERS=1 -DHAVE_UNISTD_H=1 -DDIRENT=1 
OBJS	= sui.o getopt.o getopt1.o queue.o
 
CFLAGS  = -O3 -D_Linux_ $(DEFIN) -I../lib -I./include 
ARFLAGS = r
CCOPTS  = $(CFLAGS)
LINTOPTS = $(CFLAGS)
AR      = ar
CC      = gcc
LIBTGT  = libsui.a
                       

$(LIBTGT):  $(OBJS)
	$(AR) $(ARFLAGS) $(LIBTGT) $(OBJS)
	

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CCOPTS) -c $*.c

sui.o:		sui.c sui.h
getopt.o:	getopt.c getopt.h
getopt1.o:	getopt1.c getopt1.h
queue.o:	queue.c sui.h

all:
	rm -f *.o $(LIBTGT)
	touch *
	make $(LIBTGT)

clean: 
	-rm -f *.o *.ln $(LIBTGT)
