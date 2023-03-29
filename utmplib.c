// utmplib.c - functions to buffer reads from utmp file

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <utmp.h>

#define NRECS 16
#define NULLUT ((struct utmp*)NULL)
#define UTSIZE (sizeof(struct utmp))

static char utmpbuf[NRECS * UTSIZE];	/* storage */
static int num_recs;					/* num stored */
static int cur_rec;						/* next to go */
static int fd_utmp = -1;				/* read from */

/* declare the following functions to be written below */
int utmp_open(char *filename);
struct utmp* utmp_next();
int utmp_reload();
void utmp_close();

/*
	Open the UTMP file
	return: a file descriptor of the file
*/
int utmp_open(char *filename) {
	fd_utmp = open(filename, O_RDONLY);		/* open it */
	cur_rec = num_recs = 0;					/* no recs yet */
	return fd_utmp;							/* report */
}

/*
	Access the next UTMP record if any
	return: the pointer of the next record
*/
struct utmp *utmp_next() {
	struct utmp *recp;
	if (fd_utmp == -1)								/* error? */
		return NULLUT;
	if (cur_rec == num_recs && utmp_reload() == 0)	/* any more? */
		return NULLUT;
								 /* get address of next record */
	recp = (struct utmp*)&utmpbuf[cur_rec * UTSIZE];
	cur_rec++;
	return recp;
}

/*
	Read next bunch of records into buffer
	return: how many bytes read
*/
int utmp_reload() {
	int amt_read;					/* read them in */
	amt_read = read(fd_utmp, utmpbuf, NRECS * UTSIZE);
	num_recs = amt_read / UTSIZE;	/* how many did we get? */
	cur_rec = 0;					/* reset pointer just in case */
	return num_recs;
}

void utmp_close() {
	if (fd_utmp != -1)				/* don't close if still not open */
		close(fd_utmp);
}
