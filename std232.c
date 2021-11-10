/*-------------- Linux Serial Sniffer - User Filter ------------*/
/*                                                              */
/* Reads sniffer data packets from the pipe and then formats    */
/* the output from the following:                               */
/*                                                              */
/*    000000 : <  61, 62, 63, 78, 79,       			*/
/*    000000 : >  41, 42, 43, 00,                      		*/
/*    000009 : <  67, 6b, 06,                          	 	*/
/*                                                              */
/* where ^^^ is the number of hex bytes for that direction and  */
/* the '<' is input, '>' is output direction.                   */
/*                                                              */
/* to the following format:                                     */
/*                                                              */
/*     000000 : <  61 (a), 62 (b), 63 (c), 78 (x), 79 (y),      */
/*     000000 : >  41 (A), 42 (A), 43 (C), 00 NUL,              */
/*     000009 : <  67 (g), 6b (k), 06 ACK,                      */
/*                                                              */
/* Bracketted data is the ASCII equivalent or if it is a        */
/* control charcter its nuemonic is shown instead.              */
/*                                                              */
/*								*/
/*  Generally you would use this as follows:			*/
/*								*/
/*	sniffer | std232 > myfile.txt                           */
/*								*/
/*  This will read the sniffer data, filter it, then forward    */
/*  the final data to the user file: myfile.txt for later use   */
/*								*/
/*--------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 0
#define MAXSIZE 40
#define LINEMAX 80
#define ENTRIES 256 
#define ENTRY_SIZE 4
#define ASCII  "./ascii.flt"


char cmatrix [ENTRIES+1][ENTRY_SIZE+1];
char dmatrix [] = "...\0";



char * convert (const char * temp)
{
	unsigned int index;

	(unsigned int) index = strtol(temp, NULL, 16);
	if (index < 256)
	     return (&cmatrix[index][0]);
	else return (&dmatrix[0]);
}



int main(void)
{
	char line[LINEMAX], out[LINEMAX], temp[LINEMAX], *result;
	int  x, linesize;
	FILE *fp;

	/* Load the user conversion matrix */
	fp = fopen(ASCII, "r");
	if (fp == NULL) {	
		fprintf(stderr, "Error Opening user conversion file: %s\n",
			ASCII);
		exit(-1);
	}

	/* Read the user conversion matrix from file */
	for (x = 0; x < ENTRIES; x++) {
		if (fgets(&line[0], LINEMAX, fp) == NULL) {
			perror("Bad filter read");
			exit(-1);
		}
		if (strcpy(&cmatrix[x][0], &line[0]) == NULL) {
			perror("Null value error on strcpy 1");
			exit(-1);
		}
		cmatrix[x][ENTRY_SIZE-1] = '\0';
	}	
	
	/* Close the user conversion matrix file now */
	fclose(fp);


	while (fgets(line, LINEMAX-1, stdin) != NULL)
	{
		/* Got a line now, Print out line address */
		linesize = strlen(&line[0]);
		strncpy(&out[0], &line[0], 10);
		out[10] = ' '; out[11] = ' '; out[12] = '\0';
		fprintf (stdout, "%s", &out[0]);

		/* Now printout the character conversions */
		if (linesize > 12) {
			for (x = 12; x < linesize-1 && x < MAXSIZE; x++) {
				strncpy(&temp[0], &line[x], 2);
				temp[2] = '\0';
				fprintf(stdout, "%s ", &temp[0]);
				result = convert(&temp[0]);
				fprintf(stdout, "%s, ", result);
				x += 3;		
			}
		}
		fprintf(stdout,"\n");
		fflush (stdout);		 
	}
	return (0);
}

/* End of Program */
