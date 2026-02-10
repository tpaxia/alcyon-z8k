/*
 * Archiver for Z8000 x.out object files.
 *
 * Ported for cross-compilation on macOS:
 *   - POSIX I/O replacing CP/M openb/creatb
 *   - Real fstat() instead of _filesz() hack
 *   - Byte-swapping for big-endian archive format
 *   - Standard C headers and types
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "xout.h"


/* POSIX wrappers for CP/M I/O */
static int openb(const char *name, int mode)
{
	int flags;
	switch (mode) {
	case 0:  flags = O_RDONLY; break;
	case 1:  flags = O_WRONLY; break;
	case 2:  flags = O_RDWR;   break;
	default: flags = O_RDONLY; break;
	}
	return open(name, flags);
}

static int creatb(const char *name, int mode)
{
	return open(name, O_WRONLY | O_CREAT | O_TRUNC, mode);
}


#define EVEN 0
#define ODD 1


struct ar8k_hd	head;		/* header from the old archive */

struct stat statbuf;

char	**filvec,
	*filename,
	*archive,
	filmrk[256],
	buffer[512];

char	*tmpname = "ar8kxxxx.tmp";

int	key = 0,
	verbose,
	arfile,
	tpfile,
	tmpflag = 0,
	file,
	filyet,
	filcnt;

uint16_t inmagic;


/* forward declarations */
int  settmp(void);
void wrmagic(int f);
char *tail(char *s);
void comment(char *s);
void errex(const char *s, ...);
void getfile(int f, char *s);
int  inlist(void);
void copy(int outf, int outflag, int inf, int inflag);
void skip(void);
void puthead(int f);
void append(int f);
void chmagic(int f);


int main(int argc, char **argv)
{
	int i;
	char *cp;

	if( argc < 3 ){
		printf("usage:  ar8k  key  archfile [files ...]\n");
		exit(1);
	}

	cp = argv[1];
	while( (i = *cp++) ) switch( i ){

case 'd':
case 'q':
case 'r':
case 't':
case 'x':
		if( key ) errex("only one of d q r t x allowed");
		key = i;
		continue;

case 'v':	verbose++;
		continue;

default:	errex("illegal key");
	}
	if( key == 0 ) errex("no key specified");

	archive = argv[2];
	filvec = &argv[3];
	filyet = filcnt = argc-3;
	if( filcnt > 256 ) errex( "too many file names" );

	switch( key ){

case 'd':
case 'r':
		tmpflag = settmp();
	}

	switch( key ){

case 'd':
case 'r':
case 't':
case 'x':
		arfile = openb( archive , 0 );
		if( arfile < 0 ){
			if( key != 'r' ) errex("can't read %s\n",archive );
			key = 'q';
		} else {
			chmagic( arfile );
		}
	}

	if( key == 'q' ){
		arfile = openb( archive , 2 );
		if( arfile < 0 ){
			arfile = creatb( archive , 0666 );
			if( arfile >= 0 ) wrmagic( arfile );
				else  errex("can't append to %s",archive);
		} else {
			chmagic( arfile );
			lseek( arfile , 0L , 2 );
		}
		append( arfile );
		close(arfile);
		exit(0);
	}

	while( read( arfile, (char *) &head, sizeof head ) == sizeof head ){
		swap_ar(&head);

		switch( key ){

case 'd':
			if( inlist() ){
				comment("deleting");
				skip();
			} else {
				puthead( tpfile );
				copy( tpfile , EVEN , arfile , EVEN );
			}
			break;

case 'r':
			if( inlist() && (file = openb(filename,0)) >= 0 ){
				skip();
				getfile( tpfile , "replacing");
			} else {
				puthead( tpfile );
				copy( tpfile , EVEN , arfile , EVEN );
			}
			break;

case 't':
			if( filcnt == 0 || inlist() ){
				if( verbose ){
					printf(" %7ld ", (long)head.ar8k_size );
				}
				printf("%-14.14s\n",head.ar8k_name);
			}
			skip();
			break;

case 'x':
			if( (filcnt == 0 || inlist()) &&
			    (file=creatb(head.ar8k_name,0666)) >=0 ){
				comment("extracting");
				copy( file , ODD , arfile , EVEN );
				close( file );
			} else {
				skip();
			}
			break;
		}
	}

	switch( key ){

case 'r':
		if( filyet ) append( tpfile );

case 'd':
		close( arfile );
		arfile = creatb( archive , 0666 );
		if( arfile < 0 ){
			printf("cannot create %s\n",archive );
		} else {
			lseek( tpfile , 0L , 0 );
			while( (i = read( tpfile , buffer , 512)) > 0 )
				if( write( arfile , buffer , i ) != i )
					errex("botch in recopying the archive");
		}
	}
	if( filyet != 0 ){
		printf("\nfiles not processed:\n\n");
		for( i=0; i<filcnt; i++ )
			if( !filmrk[i] )
				printf("\t%s\n",filvec[i] );
	}
	if (tmpflag) {
		close(tpfile);
		unlink(tmpname);
	}
	return 0;
}


int settmp(void)
{
	close( creatb( tmpname, 0600 ) );
	tpfile = openb( tmpname , 2 );
	if( tpfile < 0 ){
		printf("cannot create tempfile\n" );
		exit(1);
	}
	wrmagic( tpfile );
	return(1);
}

void wrmagic(int f)
{
	/* Write 2-byte magic in big-endian */
	uint16_t mag = htons(AR8KMAGIC);
	if( write( f , (char *) &mag , 2 ) != 2 )
		errex("can't write magic word\n");
}


char *tail(char *s)
{
	int i;
	char *t, *u;

	for(;;){
		u = t = s;
		while( (i = *u++) )
			if( i == '/' ) t = u;
		if( *t ) return( t );
		if( t == s ) errex("bad file name");
	}
}


void comment(char *s)
{
	if( verbose ) printf("%s:\t%-14.14s\n",s,head.ar8k_name);
}


void errex(const char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	printf("fatal:\t");
	vprintf(s, ap);
	printf("\n");
	va_end(ap);
	exit(1);
}


void getfile(int f, char *s)
{
	char *cp;
	int i;

	cp = tail( filename );
	fstat( file , &statbuf );
	lseek(file, 0L, 0);
	head.ar8k_size = statbuf.st_size;
	head.ar8k_uid = statbuf.st_uid;
	head.ar8k_gid = statbuf.st_gid;
	head.ar8k_date = statbuf.st_mtime;
	head.ar8k_mode = statbuf.st_mode;
	for( i=0; i<14; i++ )
		if( (head.ar8k_name[i] = *cp) )
			cp++;
	comment( s );
	puthead( f );
	copy( f , EVEN , file , ODD );
	close( file );
}


int inlist(void)
{
	char *cp, *bp;
	int j, i;

	for( i=0; i<filcnt; i++ ){
		if( !filmrk[i] ){
			cp = tail( filvec[i] );
			bp = head.ar8k_name;
			for( j=0; j<14; j++ ){
				if( *cp != *bp++ ) break;
				if( *cp ) cp++;
			}
			if( j == 14 ){
				filmrk[i]++;
				filyet--;
				filename = filvec[i];
				return( 1 );
			}
		}
	}
	return( 0 );
}


void copy(int outf, int outflag, int inf, int inflag)
{
	int i, j;

	i = head.ar8k_size >> 9;
	while( --i >= 0 ){
		if( read( inf , buffer , 512 ) != 512 ) errex("bad read");
		if( write( outf , buffer , 512) != 512 ) errex("bad write");
	}
	i = head.ar8k_size;
	j = i = i & 0777;
	if( i & 1 ){
		if( inflag == EVEN ) i++;
		if( outflag == EVEN ) j++;
	}
	if( i ){
		if( read( inf , buffer , i ) != i ) errex("bad read.");
		buffer[i] = 0;
		if( write( outf , buffer , j ) != j ) errex("bad write.");
	}
}


void skip(void)
{
	long size;

	size = head.ar8k_size;
	if( size & 1 ) size++;
	lseek( arfile , size , 1 );
}


void puthead(int f)
{
	/* Swap to big-endian before writing */
	struct ar8k_hd tmp = head;
	swap_ar(&tmp);
	if( write( f , (char *) &tmp , sizeof tmp ) != sizeof tmp )
		errex( "header write error" );
}


void append(int f)
{
	int i;

	for( i=0; i<filcnt; i++ ){
		if( !filmrk[i] ){
			filename = filvec[i];
			if( (file = openb(filename,0)) >= 0 ){
				filmrk[i]++;
				filyet--;
				getfile( f ,"appending");
			} else {
				printf("cannot read %s\n",filename);
			}
		}
	}
}

void chmagic(int f)
{
	uint16_t mag;
	read( f , (char *) &mag , 2 );
	if( ntohs(mag) != AR8KMAGIC ) errex("%s is not an archive",archive);
}
