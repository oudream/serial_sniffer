#	Copyright (c) 1999 Grahame M. Kelly
#
#	You may distribute and/or use for any purpose modified or unmodified
#	copies of this software if you preserve the copyright notice above.
#
#	THIS SOFTWARE IS PROVIDED AS IS AND COME WITH NO WARRANTY OF ANY
#	KIND, EITHER EXPRESSED OR IMPLIED.  IN NO EVENT WILL THE
#	COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES RESULTING FROM THE
#	USE OF THIS SOFTWARE.

CC	= gcc
CFLAGS	= -O2 -Wall

all:		sniffer std232

sniffer:	sniffer.h sniffer.c
		${CC} ${CFLAGS} -o sniffer sniffer.c

std232:		std232.c 
		${CC} ${CFLAGS} -o std232  std232.c


