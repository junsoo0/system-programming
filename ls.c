/*
	ls.c
		- purpose: list contents of files and directories
		- action: if no args, use "." else list files in args
		- note: uses stat and pwd.h and grp.h
		- options:
			-S: sort by file size, largest first 
			-s: sort by file size, smallest first
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

#define MAX_ELEMENTS 200
typedef enum sort_type {
	NONE, DESC, ASC
} sort_type;

typedef struct {
	char *filename;
	int fsize;
} element;

void do_ls(char dirname[], sort_type type);
void dostat(char *filename);
void show_file_info(char *filename, struct stat *info_p);
void mode_to_letters(int mode, char str[]);
char *uid_to_name(uid_t uid);
char *gid_to_name(gid_t gid);

void push(element item, sort_type t);
element pop(sort_type t);

element heap[MAX_ELEMENTS];
int sz = 0;

int main(int argc, char *argv[]) {
	sort_type t = NONE;
	char *file_list[MAX_ELEMENTS];
	int file_cnt = 0;
	int i;

	while(--argc) {
		argv++;
		if((*argv)[0] == '-') {
			if((*argv)[1] == 'S')
				t = DESC;
			else if((*argv)[1] == 's')
				t = ASC;
		}
		else
			file_list[file_cnt++] = *argv;
	}

	if(file_cnt == 0)
		do_ls(".", t);
	else {
		for(i = 0; i < file_cnt; i++) {
			printf("%s:\n", file_list[i]);
			do_ls(file_list[i], t);
		}
	}

	return 0;
}

void do_ls(char dirname[], sort_type t) {
	/*
	   list files in directory called dirname
	*/
	DIR *dir_ptr;				/* the directory */
	struct dirent *direntp;		/* each entry */
	element temp;
	struct stat info;
	char str[500];
	if((dir_ptr = opendir(dirname)) == NULL)
		fprintf(stderr, "ls2: cannot open %s\n", dirname);
	else {
		if(t == NONE) {
			while((direntp = readdir(dir_ptr)) != NULL) {
				if (strcmp(dirname, ".") == 0)
					dostat(direntp->d_name);
				else {
					sprintf(str, "%s/%s", dirname, direntp->d_name);
					dostat(str);
				}
			}
		}
		else if(t == DESC || t == ASC) {
			while((direntp = readdir(dir_ptr)) != NULL) {
				temp.filename = direntp->d_name;
				if (strcmp(dirname, ".") == 0)
					stat(temp.filename, &info);
				else {
					sprintf(str, "%s/%s", dirname, direntp->d_name);
					stat(str, &info);
				}
				temp.fsize = info.st_size;
				push(temp, t);
			}
			while(sz > 0) {
				temp = pop(t);
				if (strcmp(dirname, ".") == 0)
					dostat(temp.filename);
				else {
					sprintf(str, "%s/%s", dirname, temp.filename);
					dostat(str);
				}
			}
		}
	}
}

void dostat(char *filename) {
	struct stat info;
	if(stat(filename, &info) == -1)		/* cannot stat */
		perror(filename);				/* say why */
	else
		show_file_info(filename, &info);	/* else show who */
}

void show_file_info(char *filename, struct stat *info_p) {
	/*
	   display the info about 'filename'
	   the info is stored in struct at info_p
	*/
	char *uid_to_name(), *ctime(), *gid_to_name(), *filemode();
	void mode_to_letters();
	char modestr[] = "----------";

	mode_to_letters(info_p->st_mode, modestr);
	printf("%s", modestr);
	printf("%4d ", (int)info_p->st_nlink);
	printf("%-8s ", uid_to_name(info_p->st_uid));
	printf("%-8s ", gid_to_name(info_p->st_gid));
	printf("%-8ld ", (long)info_p->st_size);
	printf("%.12s ", 4+ctime(&info_p->st_mtime));
	printf("%s\n", filename);
}

void mode_to_letters(int mode, char str[]) {
	if(S_ISDIR(mode)) str[0] = 'd';	/* directory? */
	if(S_ISCHR(mode)) str[0] = 'c';	/* char devices? */
	if(S_ISBLK(mode)) str[0] = 'b';	/* block devices? */

	if(mode & S_IRUSR) str[1] = 'r'; /* 3 bits for user */
	if(mode & S_IWUSR) str[2] = 'w'; 
	if(mode & S_IXUSR) str[3] = 'x';

	if(mode & S_IRGRP) str[4] = 'r'; /* 3 bits for group */
	if(mode & S_IWGRP) str[5] = 'w';
	if(mode & S_IXGRP) str[6] = 'x';

	if(mode & S_IROTH) str[7] = 'r'; /* 3 bits for other */
	if(mode & S_IWOTH) str[8] = 'w';
	if(mode & S_IXOTH) str[9] = 'x';
}

char *uid_to_name(uid_t uid) {
	/*
	   returns pointer to username associated with uid, uses getpwuid()
	*/
	struct passwd *getpwuid(), *pw_ptr;
	static char numstr[10];

	if((pw_ptr = getpwuid(uid)) == NULL) {
		sprintf(numstr, "%d", uid);
		return numstr;
	}
	else
		return pw_ptr->pw_name;
}

char *gid_to_name(gid_t gid) {
	/*
	   returns pointer to group number gid, uses getgrgid()
	*/
	struct group *getgrgid(), *grp_ptr;
	static char numstr[10];
    
	if((grp_ptr = getgrgid(gid)) == NULL) {
			sprintf(numstr, "%d", gid);
			return numstr;
	}
	else
		return grp_ptr->gr_name;
}

void push(element item, sort_type t) {
	int idx;

	if(sz == MAX_ELEMENTS - 1) {
		fprintf(stderr, "The heap is full.\n");
		exit(EXIT_FAILURE);
	}

	idx = ++sz;
	while(idx != 1) {
		if(t == DESC && item.fsize <= heap[idx/2].fsize)
			break;
		else if(t == ASC && item.fsize >= heap[idx/2].fsize)
			break;
		heap[idx] = heap[idx/2];
		idx /= 2;
	}
	heap[idx] = item;
}

element pop(sort_type t) {
	int parent, child;
	element item, temp;

	if (sz == 0) {
		fprintf(stderr, "The heap is empty.\n");
		exit(EXIT_FAILURE);
	}

	item = heap[1];
	temp = heap[sz--];
	parent = 1;
	child = 2;

	while(child <= sz) {
		if(t == DESC) {
			if((child < sz) && (heap[child].fsize < heap[child+1].fsize))
				child++;
			if(temp.fsize >= heap[child].fsize)
				break;
		}
		else if(t == ASC) {
			if((child < sz) && (heap[child].fsize > heap[child+1].fsize))
				child++;
			if(temp.fsize <= heap[child].fsize)
				break;
		}

		heap[parent] = heap[child];
		parent = child;
		child *= 2;
	}
	heap[parent] = temp;
    
	return item;
}
