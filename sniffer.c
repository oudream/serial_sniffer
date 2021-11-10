/*
   Copyright (c) 1999 Grahame M. Kelly (gmkelly@zip.com.au)

   You may distribute and/or use for any purpose modified or unmodified
   copies of this software if you preserve the copyright notice above.

   THIS SOFTWARE IS PROVIDED AS IS AND COME WITH NO WARRANTY OF ANY
   KIND, EITHER EXPRESSED OR IMPLIED.  IN NO EVENT WILL THE
   COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES RESULTING FROM THE
   USE OF THIS SOFTWARE.

   Serial Sniffer System for reverse engineering the Nikon E950
   Digitial Camera Protocols so that the GNU GPhoto system may
   be used under Linux OS.

   Revision     Date            Comments
   --------     ----            ---------------------------------------

   05		02/10/99	Modified for User Filter Operation
   00           26/09/99        Initial 

 */

#include "sniffer.h"

unsigned int IN_count = 0, OUT_count = 0;
char showchar[7] = "(X), \0\0";
char showcont[7] = "CNT, \0\0";
static char last_direction = '@';
static int LineCount = LineMAX + 1, STOP = FALSE;
static void sig_usr (int);	/* Signal Handler */
pid_t	pid;	

int 
main ()
{
  int fdA, fdB, cC, iresult = 0, presult = 0;
  struct termios oldtioA, newtioA, oldtioB, newtioB;
  byte bufferA[BUFFSIZE], bufferB[BUFFSIZE];
  fd_set readfds, problem, speedy1, speedy2;
  int fdes[2];			/* Pipe */
  int maxfd;
  struct ppack PipePackA, PipePackB;
  struct ppack *PPackA = &PipePackA, *PPackB = &PipePackB;

  for (cC = 0; cC < BUFFSIZE - 1; cC++)
    bufferA[cC] = bufferB[cC] = 0x00;
  for (cC = 0; cC < PipeBuffSize; cC++)
    {
      PPackA->PBuff[cC] = 0x00;
      PPackB->PBuff[cC] = 0x00;
    }
  printf ("\nLinux Serial Sniffer Version %s.%s\n", REV_Major, REV_Minor);
  printf ("Camera Input sniffed on (Device 1) serial line: %s\n", DEVICE1);
  printf ("Camera Output sniffed on (Device 2) serial line: %s\n", DEVICE2);
  if (DEBUG) printf("Debugging Active\n\n"); else printf("\n");


#if TYPE2
  fdA = open (DEVICE2, O_RDWR | O_NOCTTY | O_NONBLOCK | O_SYNC);
  if (fdA < 0)
    {
      perror ("Device 1 - Error ");
      exit (-1);
    }
  fdB = open (DEVICE1, O_RDWR | O_NOCTTY | O_NONBLOCK | O_SYNC);
  if (fdB < 0)
    {
      perror ("Device 2 - Error ");
      exit (-1);
    }
#else
  fdA = open (DEVICE2, O_RDONLY | O_NOCTTY);
  if (fdA < 0)
    {
      perror ("Device 1 - Error ");
      exit (-1);
    }
  fdB = open (DEVICE1, O_RDONLY | O_NOCTTY);
  if (fdB < 0)
    {
      perror ("Device 2 - Error ");
      exit (-1);
    }
#endif

  tcgetattr (fdA, &oldtioA);	/* save current port settings */
  tcgetattr (fdB, &oldtioB);	/* save current port settings */

  bzero (&newtioA, sizeof (newtioA));
  bzero (&newtioB, sizeof (newtioB));

  newtioB.c_cflag = newtioA.c_cflag = CS8 | CLOCAL | CREAD;
  newtioB.c_iflag = newtioA.c_iflag = IGNPAR;
  newtioB.c_oflag = newtioA.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtioB.c_lflag = newtioA.c_lflag = 0;
  newtioB.c_cc[VTIME] = newtioA.c_cc[VTIME] = 0;
  newtioB.c_cc[VMIN] = newtioA.c_cc[VMIN] = 1;

  /* Set new speeds */
  cfsetispeed (&newtioA, BAUDRATE);
  cfsetospeed (&newtioA, BAUDRATE);
  cfsetispeed (&newtioB, BAUDRATE);
  cfsetospeed (&newtioB, BAUDRATE);

  tcsetattr (fdA, TCSANOW, &newtioA);
  tcsetattr (fdB, TCSANOW, &newtioB);

  tcflush (fdA, TCIFLUSH);
  tcflush (fdB, TCIFLUSH);


  /* Make the Pipe the primary task to write to, secondary to read from */
  presult = pipe (&fdes[0]);
  if (presult < 0)
    {
      perror ("Pipe Create Error ");
      exit (-1);
    }
  if (DEBUG)
    fprintf (stderr, "Pipe fd = %d, %d\n", fdes[0], fdes[1]);

  switch (pid = fork ())
    {
    case 0:			/* we are the child */
      close (fdes[1]);		/* Close the Write Pipe */
      piper (fdes[0]);		/* Call the Piper ----  */
      break;

    case -1:
      fprintf (stderr, "fork failed.\n");
      exit (-1);
      break;

    default:			/* we are the parent */
      close (fdes[0]);		/* Close the Read Pipe */
      break;
    }

  if (DEBUG)
    fprintf (stderr, "PARENT: The child PID %d, Write Pipe %d\n", pid, fdes[1]);

  if (signal (SIGUSR1, sig_usr) == SIG_ERR)
    {
      perror ("\nUser signal received OK");
      kill(pid, SIGKILL);
      STOP = TRUE;
    }

  if (fdA > fdB)
     maxfd = fdA + 1;
  else
     maxfd = fdB + 1;

  FD_ZERO (&readfds);
  FD_ZERO (&problem);
  FD_SET (fdA, &readfds);
  FD_SET (fdA, &problem);
  FD_SET (fdB, &readfds);
  FD_SET (fdB, &problem);

  /* Now make a "master" copy of fds for fast switching */
  speedy1 = readfds; speedy2 = problem;

  while (!STOP)
    {

      readfds = speedy1; problem = speedy2;
      iresult = select (maxfd, &readfds, NULL, &problem, NULL);
      if (iresult < 0)
	{
	  perror ("Select on Readfds ");
	  exit (-1);
	}
      if (FD_ISSET (fdA, &readfds))
	{
	  PPackA->count = read (fdA, &PPackA->PBuff[0], PipeBuffSize - 1);
	  if (PPackA->count < 0)
	    {
	      perror ("READ on fdA ");
	      exit (-1);
	    }
	  PPackA->direction = CAMERA_IN;
	  presult = write (fdes[1], &PipePackA, sizeof (PipePackA));
	  if (presult < 0)
	    {
	      perror ("Pipe Write A ");
	      exit (-1);
	    }
	  if (TYPE2)
	    {
	      /* Send Character back out again || install your adds here */
	      presult = write (fdA, &PPackA->PBuff[0], PPackA->count);
	      if (presult != PPackA->count)
		{
		  perror ("Resend fdA error ");
		  exit (-1);
		}
	    }
	}
      if (FD_ISSET (fdB, &readfds))
	{
	  PPackB->count = read (fdB, &PPackB->PBuff[0], PipeBuffSize - 1);
	  if (PPackB->count < 0)
	    {
	      perror ("READ on fdB ");
	      exit (-1);
	    }
	  PPackB->direction = CAMERA_OUT;
	  presult = write (fdes[1], &PipePackB, sizeof (PipePackB));
	  if (presult < 0)
	    {
	      perror ("Pipe Write B ");
	      exit (-1);
	    }
	  if (TYPE2)
	    {
	      /* Send Character back out again || install your adds here */
	      presult = write (fdB, &PPackB->PBuff[0], PPackB->count);
	      if (presult != PPackB->count)
		{
		  perror ("Resend fdB error ");
		  exit (-1);
		}
	    }
	}
      if (FD_ISSET (fdA, &problem))
	{
	  fprintf (stderr, "\n\aProblem on fdA input\n");
	}
      if (FD_ISSET (fdB, &problem))
	{
	  fprintf (stderr, "\n\aProblem on fdB input\n");
	}
      if (iresult < 1)
	fprintf (stderr, "\nShould NOT get here...\a\n");
  
  }
  tcsetattr (fdA, TCSANOW, &oldtioA);	/* Restore ttyA */
  tcsetattr (fdB, TCSANOW, &oldtioB);	/* Restore ttyB */
  close (fdA);
  close (fdB);
  close (fdes[1]);
  kill(pid, SIGKILL);
  return (0);
}
/* End of Main */


static void 
sig_usr (int signo)
{
  /* User can terminate the above process */
  /* by using a Control C at the keyboard */ 
  if (signo == SIGINT)
    STOP = TRUE;
    kill(pid, SIGKILL);
  return;
}



/*------------------- Piper Program ----------------------------*/
/*                                                              */
/* Reads sniffer data packets from the pipe and then formats    */
/* the output in accordance to the following design:            */
/*                                                              */
/*      000000 : <  61, 62, 63, 78, 79,       			*/
/*      000000 : >  41, 42, 43,                       		*/
/*      000009 : <  67, 6b,                           	 	*/
/*      000005 : >  00, 01, 02,                       		*/
/*      000013 : <  0a, 0b,                                	*/
/*                                                              */
/* where ^^^ is the number of bytes for that direction and      */
/* the '<' is input, '>' is output direction.                   */
/*                                                              */
/* Bracketted data is the ASCII equivalent or if it is a        */
/* control charcter its nuemonic is shown instead.              */
/*                                                              */
/*--------------------------------------------------------------*/


int 
user_format (char direction, byte * Obuffer, int bytes_read)
{
  int Gcount, Pcount, Sx;
  static int *pcount;

  Sx = 0;
  while (bytes_read)
    {
      if (direction == '<')
	pcount = &IN_count;
      else
	pcount = &OUT_count;

      if (direction != last_direction)
	{
	  last_direction = direction;
	  fprintf (stdout, "\n%06x : %c  ", *pcount, direction);
	  LineCount = 1;
	}
      if (direction == '>')
	{
	  if (LineCount > LineMAX)
	    {
	      fprintf (stdout, "\n%06x : >  ", *pcount);
	      LineCount = 1;
	    }
	}
      if (direction == '<')
	{
	  if (LineCount > LineMAX)
	    {
	      fprintf (stdout, "\n%06x : <  ", *pcount);
	      LineCount = 1;
	    }
	}

      Pcount = Gcount = bytes_read;
      /* Print the Hex data */
      while (Gcount--)
	{
	  if (LineCount > LineMAX)
	    {
	      fprintf (stdout, "\n%06x : %c  ", *pcount, last_direction);
	      LineCount = 1;
	    }
          /* Should put out characters here I think */
          fprintf(stdout, "%02x, ", Obuffer[Sx++]);
	  *pcount += 1;
	  LineCount++;
	}
      bytes_read = bytes_read - Pcount;
    }
  return 0;
}


int 
piper (int fdD)
{
  int result;
  struct ppack RecPacket, *RPack = &RecPacket;

  if (DEBUG) fprintf (stderr,\
	"\nCHILD: Piper Formatter Read Pipe %d\n", fdD);


  while (!STOP)
    {
      result = read (fdD, RPack, sizeof (RecPacket));
      if (result != sizeof (RecPacket))
	{
	  perror ("Pipe Read Error - A");
	  STOP = TRUE;
	  exit (-1);
	}
      user_format (RPack->direction, &RPack->PBuff[0], RPack->count);
      fflush(stdout);
    }
  close (fdD);
  exit (0);
}

/* End of Program */
