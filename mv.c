/*
	mv.c
		- mv for file extensions
		- usage: ./mv -d dest_directory -e file_extension
		- options:
			-d: the directory flag
			-e: the file extension flag
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#define BUFFERSIZE 4096
#define COPYMODE 0644
#define TRUE 1
#define FALSE 0

void oops(char *s1, char *s2);

int main(int argc, char *argv[]) {
	int in_fd, out_fd, n_chars, f_ext_pos;
	char buf[BUFFERSIZE];
	DIR *dir_ptr;
	struct dirent *direntp;
	char *dir, f_ext[10], *src_file, dst_file[100];
	int i;
	int dir_check = FALSE, f_ext_check = FALSE, dir_find = FALSE, f_ext_find = FALSE;

	/* check args */
	for (i = 1; i <= 2; i++) {
		if (argc >= 2 * i + 1) {
			if (strcmp(argv[2 * i - 1], "-d") == 0) {
				dir = argv[2 * i];
				dir_check = TRUE;
			}
			else if (strcmp(argv[2 * i - 1], "-e") == 0) {
				strcpy(f_ext, ".");
				strcat(f_ext, argv[2 * i]);
				f_ext_check = TRUE;
			}
		}
	}

	if (dir_check == FALSE) {
		fprintf(stderr, "Destination directory does not exist\n");
		exit(1);
	}
	else if (f_ext_check == FALSE) {
		fprintf(stderr, "Missing file extension\n");
		exit(1);
	}

	if (argc >= 6) {
		fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], argv[5][1]);
		printf("Usage: %s -d destination_directory -e extension\n", argv[0]);
		exit(1);
	}

	dir_ptr = opendir(".");
	while ((direntp = readdir(dir_ptr)) != NULL) {
		if (strcmp(dir, direntp->d_name) == 0)
			dir_find = TRUE;
	}
	rewinddir(dir_ptr);

	if (dir_find == FALSE) {
		fprintf(stderr, "Destination directory does not exist\n");
		exit(1);
	}

	while ((direntp = readdir(dir_ptr)) != NULL) {
		src_file = direntp->d_name;
		f_ext_pos = strlen(src_file) - strlen(f_ext);
		if (f_ext_pos < 0 || strcmp(&src_file[f_ext_pos], f_ext) != 0)
			continue;
		
		f_ext_find = TRUE;
		/* open files */
		in_fd = open(src_file, O_RDONLY);
		if (in_fd == -1)
			oops("Cannot open", src_file);

		strcpy(dst_file, dir);
		strcat(dst_file, "/");
		strcat(dst_file, src_file);

		out_fd = creat(dst_file, COPYMODE);
		if (out_fd == -1)
			oops("Cannot creat", dst_file);
		
		/* copy files */
		while ((n_chars = read(in_fd, buf, BUFFERSIZE)) > 0)
			if (write(out_fd, buf, n_chars) != n_chars)
				oops("write error to", dst_file);
		if (n_chars == -1)
			oops("read error from", src_file);

		/* close files */
		if (close(in_fd) == -1 || close(out_fd) == -1)
			oops("error closing file", "");

		remove(src_file);
	}
	closedir(dir_ptr);

	if (f_ext_find == FALSE) {
		fprintf(stderr, "No files with extension %s found\n", &f_ext[1]);
		exit(1);
	}

	return 0;
}

void oops(char *s1, char *s2) {
	fprintf(stderr, "error: %s ", s1);
	perror(s2);
	exit(1);
}
