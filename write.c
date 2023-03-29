/*
	write.c
		- purpose: send messages to another terminal
		- method: open the other terminal for output then
				  copy from stdin to that terminal
		- shows: a terminal is just a file supporting regular I/O
		- usage: ./write ttyname
*/

#include <stdio.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
	int fd;
	char buf[BUFSIZ];

	/* check args */
	if (argc != 2) {
		fprintf(stderr, "usage: %s ttyname\n", argv[0]);
		exit(1);
	}

	/* open devices */
	fd = open(argv[1], O_WRONLY);
	if (fd == -1) {
		perror(argv[1]);
		exit(1);
	}

	/* loop until EOF on input */
	while (fgets(buf, BUFSIZ, stdin) != NULL)
		if (write(fd, buf, strlen(buf)) == -1)
			break;
	close(fd);
}
