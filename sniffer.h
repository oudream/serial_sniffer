/*      Serial Sniffer Header File

   Copyright (c) 1999 Grahame M. Kelly (gmkelly@zip.com.au)

   You may distribute and/or use for any purpose modified or unmodified
   copies of this software if you preserve the copyright notice above.

   THIS SOFTWARE IS PROVIDED AS IS AND COME WITH NO WARRANTY OF ANY
   KIND, EITHER EXPRESSED OR IMPLIED.  IN NO EVENT WILL THE
   COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES RESULTING FROM THE
   USE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define CAMERA_IN	'<'
#define CAMERA_OUT	'>'


/* Type 1 PLUG is where we don't echo char but just listen  */
/* Type 2 PLUG is where we have to input and then echo char */
#define TYPE2  FALSE

#if	TYPE2
#define TYPE1  FALSE
#else
#define TYPE1  TRUE
#endif

#define LAPTOP TRUE
#define DEBUG TRUE

#define REV_Major "0\0"
#define REV_Minor "5\0"

#define BUFFSIZE 256
#define SSIZE  BUFFSIZE-1
#define BAUDRATE B115200
#define LineMAX 8
#define PipeBuffSize 16
#define SOH '#'
#define EOH '@'

#if LAPTOP
#define DEVICE1 "/dev/ttyS0"
#define DEVICE2 "/dev/ttyS1"
#else
#define DEVICE1 "/dev/ttyR4"
#define DEVICE2 "/dev/ttyR5"
#endif

#define _POSIX_SOURCE 1		/* POSIX compliant source */

typedef unsigned char byte;
typedef int boolean;
typedef void Sigfunc (int);
#if defined(SIG_IGN) && !defined(SIG_ERR)
#defined SIG_ERR ((Sigfunc *)-1)
#endif

Sigfunc *signal_intr (int, Sigfunc *);
int piper (int fdD);
char *Convert (byte inchar);
int user_format (char direction, byte * Obuffer, int bytes_read);


struct ppack
  {
    char direction;
    int count;
    byte PBuff[PipeBuffSize];
  };
