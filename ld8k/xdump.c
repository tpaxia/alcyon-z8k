/*
 * Program to dump x.out files.
 *
 * Ported for cross-compilation on macOS:
 *   - POSIX I/O replacing CP/M openb
 *   - Byte-swapping for big-endian x.out format
 *   - Fixed K&R C syntax
 *   - Standard C headers
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "xout.h"

struct x_hdr x_hdr;
struct x_sg x_sg[10];
struct x_rel x_rel;
struct x_sym x_sym;
short	sbuf[2048];

char *segtypes[] = {
	"unknown",
	"bss",
	"stack",
	"code",
	"constant pool",
	"initialized data",
	"mixed code/data, not protectable",
	"mixed code/data, protectable"
};

int	floc,
	bigs,
	infd,
	dflag = 1,
	rflag = 1,
	sflag = 1;

void doargs(char *s);
void puth(int n);
void eof(void);

int main(int argc, char **argv)
{
	int i, j, k, l;
	unsigned len, offset;

	infd = 0;  /* default: stdin */
	i = 1;
	if( i < argc && argv[i][0] == '-' ) doargs( argv[i++] );
	if( i < argc ){
		infd = open( argv[i], O_RDONLY );
		if( infd < 0 ){
			fprintf(stderr,"cannot open '%s'\n",argv[i] );
			exit(1);
		}
	}
	read( infd , &x_hdr , sizeof( x_hdr ) );
	swap_hdr( &x_hdr );
	if( !bigs )fprintf( stdout,
		"magic = %x nseg = %d init = %ld reloc = %ld symb = %ld\n",
		(unsigned)x_hdr.x_magic & 0xffff,
		x_hdr.x_nseg,
		(long)x_hdr.x_init,
		(long)x_hdr.x_reloc,
		(long)x_hdr.x_symb );
	floc = sizeof( x_hdr );
	for( i=0; i<x_hdr.x_nseg; i++ ){
		read( infd , &x_sg[i] , sizeof( x_sg[0] ) );
		swap_sg( &x_sg[i] );
		if( !bigs )fprintf( stdout,
			"%4x sg[%d]:  sgno = %d  typ = %d  len = %u\n",
			floc,
			i,
			x_sg[i].x_sg_no & 0377,
			x_sg[i].x_sg_typ & 0377,
			x_sg[i].x_sg_len );
		floc += sizeof( x_sg[0] );
	}
	for( i=0; i<x_hdr.x_nseg; i++ ){
		k =  x_sg[i].x_sg_typ;
		if( !bigs )
			fprintf( stdout, "segment %d\ttype is %s\n",
				i, segtypes[k]);
		if( k == X_SG_BSS || k == X_SG_STK ){
			if( !bigs )fprintf(stdout,"\nNo initialization");
		} else {
			len = x_sg[i].x_sg_len;
			offset = 0;
			while(len>0) {
				l = (len>4096)?4096:len;
				j = read( infd, sbuf, l );
				if( j != l ){
					fprintf(stdout,
					"unexpected eof: length is %u\n",
						offset + j);
					j = 0;
				}
				for( k = 0; k<j; offset += 2, k += 2 ){
					if( dflag ){
						if( (k & 017) == 0 )
						    fprintf(stdout,"\n%4x  %5d  %4x  ",
							floc,offset,offset);
						/* read big-endian word from buffer */
						puth( rd16be((char*)&sbuf[ k>>1 ]) );
					}
					floc += 2;
				}
			len -= l;
			}
		}
		if( dflag ) fprintf(stdout,"\n\n" );
	}
	if(x_hdr.x_reloc && rflag && !bigs)
		fprintf(stdout,"floc  sgn flg   loc   bas\n");
	for( i=0; i<x_hdr.x_reloc; i += sizeof( x_rel ) ){
		if( read( infd , &x_rel, sizeof( x_rel ) ) != sizeof(x_rel)) eof();
		swap_rel( &x_rel );
		if( rflag ) fprintf(stdout,
				"%4x %4d%4d%6d%6d\n",
				floc,
				x_rel.x_rl_sgn & 0377,
				x_rel.x_rl_flg & 0377,
				x_rel.x_rl_loc,
				x_rel.x_rl_bas );
		floc += sizeof( x_rel );
	}
	if( sflag || bigs ){
		if( !bigs )fprintf(stdout,"\nsymbols: \n\n");
		for( i=0; i<x_hdr.x_symb; i+= sizeof( x_sym ) ){
			if(read(infd, &x_sym, sizeof(x_sym)) != sizeof( x_sym ))
					eof();
			swap_sym( &x_sym );
			if( !bigs )fprintf(stdout,
				"%4x %4d%4d",
				floc,
				x_sym.x_sy_sg & 0377,
				x_sym.x_sy_fl & 0377);
			fprintf(stdout,"%6x %.8s\n",
				x_sym.x_sy_val,
				x_sym.x_sy_name );
			floc += sizeof( x_sym );
		}
	}
	return 0;
}


void doargs(char *s)
{
	sflag = dflag = rflag = 0;
	for(;;) switch( *s++ ){

case 0:		return;
case 'd':	dflag++;  continue;
case 'r':	rflag++;  continue;
case 'S':	bigs++;
case 's':	sflag++;  continue;

	}
}


char hextab[] = "0123456789abcdef";

void puth(int n)
{
	int i;

	putchar( ' ' );
	for( i=12; i>= 0; i -= 4 ) putchar( hextab[ (n>>i) & 017 ] );
}


void eof(void)
{
	fprintf(stdout,"\n\n*****unexpected eof*****\n");
	exit(0);
}
