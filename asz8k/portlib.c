/*
 * portlib.c - Unix shims for CP/M-specific functions used by asz8k.
 */
#include <stdio.h>
#include <fcntl.h>

/*
 * opena - Open a file for reading (text/ascii mode on CP/M).
 * On Unix, just open for reading.
 */
int
opena(file, mode)
char *file;
int mode;
{
	return open(file, O_RDONLY);
}

/*
 * openb - Open a file in binary mode.
 * mode==2 means read/write, otherwise read-only.
 */
int
openb(file, mode)
char *file;
int mode;
{
	return open(file, mode == 2 ? O_RDWR : O_RDONLY);
}

/*
 * creatb - Create a file in binary mode.
 */
int
creatb(file, perm)
char *file;
int perm;
{
	return open(file, O_CREAT | O_WRONLY | O_TRUNC, perm);
}

/*
 * _fopen - CP/M fopen wrapper.  The ascii flag is CP/M-only; ignored here.
 */
FILE *
_fopen(file, mode, ascii)
char *file;
char *mode;
int ascii;
{
	return fopen(file, mode);
}

/*
 * chain - CP/M program chaining to xcon linker.
 * On Unix, we just exit successfully.
 */
void
chain(argv)
char **argv;
{
	exit(0);
}
