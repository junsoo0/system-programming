// who.c - who with buffered reads using 'utmplib'

#include <stdio.h>
#include <utmp.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define SHOWHOST 1

void show_info(struct utmp* utbufp);
void show_time(long timeval);

int main() {
	struct utmp *utbufp;		/* holds pointer to next rec */
	struct utmp *utmp_next();	/* returns pointer to next */

	/* open the UTMP file */
	if(utmp_open(UTMP_FILE) == -1) {
		perror(UTMP_FILE);
		exit(1);
	}

	/* start to read an UTMP record from the UTMP file */
	while((utbufp = utmp_next()) != ((struct utmp*)NULL))
		show_info(utbufp);

	utmp_close();	/* close the opened UTMP file */
	return 0;
}

/*
	displays contents of the utmp struct in human readable form
	displays nothing if a record has no user name
*/
void show_info(struct utmp* utbufp) {
	if(utbufp->ut_type != USER_PROCESS)		/* users only */
		return;

	printf(" %-8.8s ", utbufp->ut_name);	/* login name */
	printf(" %-8.8s ", utbufp->ut_line);	/* tty */
	show_time(utbufp->ut_time);				/* display time */
	printf(" ");
	
#ifdef SHOWHOST
	printf(" (%s)", utbufp->ut_host);	/* host */
#endif
	puts("");	/* new line */
}

/*
	displays time in a format fit for human consumption
	uses ctime to build a string then pick parts out of it
	note: %12.12s prints a string with 12 chars wide and limits it to 12 chars
*/
void show_time(long timeval) {
	char* cp;					/* to hold address of time */
	cp = ctime(&timeval);		/* convert time to a string that looks like */
								/* Mon Feb 4 00:46:40 EST 2017 */
	printf("%12.12s", cp + 4);	/* pick 12 chars from pos 4 */
}
