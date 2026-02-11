/*
 * Z8000 linker -- uses x.out format modules as input and output.
 * See ld8k.man for details.
 *
 * Ported for cross-compilation on macOS (little-endian, LP64).
 * Changes from original:
 *   - POSIX I/O (open/close/read/write/lseek) replacing CP/M openb/creatb/opena
 *   - Fixed-width types in xout.h for correct on-disk struct sizes
 *   - Byte-swapping for big-endian x.out format on little-endian host
 *   - Standard C headers
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "xout.h"

#define uns unsigned int
#define reg register

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


/* this is the Z8000 loader -- it uses as input and output
   x.out format modules -- see ld8k.man for details */

#define MODNO 256	/* max allowable modules per load */
#define MAPNO 512	/* max map entries per load */
#define SYMPNO 8192	/* total number of symbols in all modules */
#define SEGNO 128	/* max number of segments per module */
#define FILNO 128	/* max number of files to process */
#define POSNO 256	/* max number of items in libraries */
#define SYMNO 2048	/* max number of symbols (total!) */
#define SEGHASH 128	/* max number of segment names */
#define SYMHASH 1024	/* max number of external symbols */

#define AR8SIZ sizeof( struct ar8k_hd )
#define XHSIZ sizeof( struct x_hdr )
#define RLC 64
#define RSIZ (RLC * sizeof( struct x_rel ))

#define islower(c) ((c)>= 'a' && (c)<= 'z')
#define isupper(c) ((c)>= 'A' && (c)<= 'Z')
#define isalpha(c) ( islower(c) || isupper(c) )
#define isdigit(n) ((n)>= '0' && (n)<= '9')
#define oktyp(old,new) (ty[((old)<<3)+(new)] & OK)
#define newtyp(old,new) (ty[((old)<<3)+(new)] & 07)
#define OK 0200
#define X_SG_UND 0

#define round(x) if((x)&1)(x)++


/* now we define the data structures we will use */

struct ar8k_hd	ar8k_hd;		/* header from the old archive */

/* segment map structures */

short	modmap[MODNO],
	modsym[MODNO],
	modno;

char	map[MAPNO];
short	symap[SYMPNO];

short	mapx,
	maps,
	symx,
	syms;


struct segtab {
	long	segsiz;
	long	segpos;
	uns	hiwat;
	uns	segrel;
	char	segtyp;
	char	segalloc;
}
	segtab[SEGNO];


short	seghash[SEGHASH];
short	symhash[SYMHASH];

struct x_hdr	x_hdr;
struct x_sg	x_sg[SEGNO];

struct filtab {
	char	*filname;
	short	posndx;
}
	filtab[FILNO];
short	filndx;

long	postab[POSNO];
short	posmax;


/* the main symbol table */

struct x_sym symtab[SYMNO];

short	symlev = 1,
	symtop;

struct x_rel	rlibuf[RLC],
		rlobuf[RLC];

/* the type clash and mixing table */

char	ty[] = {
	OK+X_SG_UND, OK+X_SG_BSS, OK+X_SG_STK, OK+X_SG_COD,
	OK+X_SG_CON, OK+X_SG_DAT, OK+X_SG_MXU, OK+X_SG_MXP,
	OK+X_SG_BSS, OK+X_SG_BSS,    X_SG_BSS,    X_SG_MXU,
	   X_SG_DAT,    X_SG_DAT, OK+X_SG_MXU,    X_SG_MXU,
	OK+X_SG_STK,    X_SG_BSS, OK+X_SG_STK,    X_SG_MXU,
	   X_SG_MXU,    X_SG_DAT, OK+X_SG_MXU,    X_SG_MXU,
	OK+X_SG_COD,    X_SG_MXU,    X_SG_MXU, OK+X_SG_COD,
	   X_SG_MXP,    X_SG_MXU,    X_SG_MXU, OK+X_SG_MXP,
	OK+X_SG_CON,    X_SG_DAT,    X_SG_DAT,    X_SG_MXP,
	OK+X_SG_CON,    X_SG_DAT, OK+X_SG_MXU, OK+X_SG_MXP,
	OK+X_SG_DAT,    X_SG_DAT,    X_SG_DAT,    X_SG_MXU,
	   X_SG_DAT, OK+X_SG_DAT, OK+X_SG_MXU,    X_SG_MXU,
	OK+X_SG_MXU, OK+X_SG_MXU, OK+X_SG_MXU, OK+X_SG_MXU,
	OK+X_SG_MXU, OK+X_SG_MXU, OK+X_SG_MXU, OK+X_SG_MXU,
	OK+X_SG_MXP,    X_SG_MXU,    X_SG_MXU, OK+X_SG_MXP,
	OK+X_SG_MXP,    X_SG_MXU,    X_SG_MXU, OK+X_SG_MXP
};

/* table marking the types with initialized data */

char	idata[] = { 0, 0, 0, 1, 1, 1, 1, 1 };

/*	control for preassigned segment types */

char	prelist[64];
int	prex;


/* miscellaneous variables */

char	*objname = "x.out",
	idbuf[32],
	libname[64],
	*fname,
	*ap,
	line[128],
	buf[512];

int	objfile,
	objifile,
	relfile,
	symfile,
	infile,
	errcnt,
	udefcnt,
	debug,
	seg,
	nonseg,
	splitid,
	shared,
	mapf,
	rlo,
	dirty,
	blen,
	strip,
	objnseg,
	nolocal,
	saverel;

uns	segrel[SEGNO],
	segsiz[SEGNO];

long	stacksiz,
	segpos[SEGNO],
	relpos,
	relval,
	relvalc,
	bpos = 1;


short	*hpos;		/* pointer into hash table for next insertion */

/* forward declarations */
void pass0(int argc, char **argv);
void pass1(void);
void p1entry(int n);
void p1load(void);
void p1doseg(void);
void p1dosym(void);
int  trysim(void);
int  symread(void);
void interlude(void);
void interpos(int i);
void loadmap(void);
void pass2(void);
void p2read(int n);
void p2load(void);
void p2dorel(void);
void reloc(struct x_rel *rlp);
void finale(void);
void finout(int i);
void dodline(char *s);
void dodset(int i, int typ);
void dodfile(char *s);
void append(char *s);
void err(const char *s, ...);
void errex(const char *s, ...);
int  findseg(int typ, uns siz);
void flshbuf(void);
void getbuf(long p);
int  idmatch(char *a, char *b);
int  install(char *s);
int  lookup(short *htab, int HTAB, char *sym);
int  mk_outfile(char *s);
void putrel(struct x_rel *rlp);
int  rdhead(void);
void setstack(char *s);
void wrrel(void);
void segdmp(char *s);
void segsho(char *s, int i);


int main(int argc, char **argv)
{
	reg int i;

	if( argc < 2 ){
		fprintf(stderr,"usage:  ld8k [flags]  file [file...]\n");
		exit(0);
	}
	for( i=0; i < MAPNO; i++ ) map[ i ] = 255;

	pass0(argc,argv);
	pass1();
	interlude();
	if( mapf ) loadmap();
	if( udefcnt && !saverel ){
		err("\n%d undefined symbols:\n\n",udefcnt);
		for( i=1; i<symlev; i++ )
			if( symtab[i].x_sy_fl == X_SY_UNX )
				fprintf(stderr,"%-8.8s\n",symtab[i].x_sy_name);
	}
	if( errcnt ) errex( "pass1" );
	objfile = mk_outfile( objname );
	if( objfile < 0 ) errex( "can't create %s" , objname );
	objifile = objfile;
	if( saverel ) relfile = mk_outfile( "relxxxx" );
	pass2();
	if( errcnt ){
		close( objfile );
		creatb( objname , 0666 );
		errex( "pass2" );
	}
	finale();
	exit( 0 );
}


void pass0(int argc, char **argv)
{
	reg int i, j;
	reg char *cp;

	for( i=1; i<argc; i++ ){
		cp = argv[i];
		if( *cp == '-' )for(;;){
			switch( *++cp ){

case 0:				break;
case 'd':			if( *++cp )	dodline( cp );
					else	dodfile( argv[ ++i ] );
				break;
case 'h':			debug++;  continue;
case 'i':			splitid++;  continue;
case 'l':			if( filndx >= FILNO ) errex( "too many files" );
				*--cp = '-';
				filtab[filndx++].filname = cp;
				break;
case 'm':			mapf++;  continue;
case 'n':			shared++;  continue;
case 'o':			objname = argv[++i];  continue;
case 'r':			saverel++;  continue;
case 't':			setstack( argv[++i] );  continue;
case 's':			strip++;  continue;
case 'u':			j = lookup(symhash,SYMHASH,argv[++i]);
				if( j == -1 ){
					*hpos = install( argv[i] );
					udefcnt++;
				}
				continue;
case 'w':			segtab[0].segtyp = X_SG_COD;
				segtab[1].segtyp = X_SG_CON;
				segtab[2].segtyp = X_SG_DAT;
				segtab[3].segtyp = X_SG_BSS;
				continue;
case 'x':			nolocal++;  continue;

default:			err( "bad flag: '%c'" , *cp );
				continue;
			}
			break;
		} else {
			if( filndx >= FILNO ) errex( "too many files" );
			filtab[ filndx ].filname = cp;
			filndx++;
		}
	}
}


void pass1(void)
{
	reg int i, j;
	reg uns k;

	for( i=0; i < filndx; i++ ){
		fname = filtab[ i ].filname;
		if( fname[ 0 ] == '-' ){
			ap = libname;
			append( "lib" );
			append( fname+2 );
			append( ".a" );
			infile = openb( libname, 0 );
			if( infile < 0 ){
				err( "can't open %s" , libname );
				continue;
			}
		} else {
			infile = openb( fname , 0 );
			if( infile < 0 ){
				err( "can't read %s" , fname );
				continue;
			}
		}
		p1entry( i );
		close( infile );
	}
	if( !saverel && udefcnt ){
		for( i=1; i<symlev; i++ ){
			if( symtab[i].x_sy_fl == X_SY_UNX &&
				(k = symtab[i].x_sy_val) != 0 ){
				j = findseg( X_SG_BSS , k );
				segtab[j].segtyp = newtyp(segtab[j].segtyp,X_SG_BSS);
				if( k > 1 ) round( segtab[j].segsiz );
				symtab[i].x_sy_val = segtab[j].segsiz;
				symtab[i].x_sy_fl = X_SY_GLB;
				symtab[i].x_sy_sg = j;
				segtab[j].segsiz += k;
				udefcnt--;
			}
		}
	}
	for( i=0; i<SEGNO; i++ ) round( segtab[i].segsiz );
	if( stacksiz ){
		j = findseg( X_SG_STK, stacksiz );
		segtab[j].segtyp = newtyp(segtab[j].segtyp,X_SG_STK);
		segtab[j].segsiz += stacksiz;
	}
}


void p1entry(int n)
{
	reg int i;
	long fpos;

	if( !rdhead() ) return;
	if( (uint16_t)x_hdr.x_magic == AR8KMAGIC ){  /* an archive! */
		int changed, pass=0;
		if( udefcnt == 0 ) return;
		do {
			if(debug) fprintf(stderr,"archive pass %d, udefcnt=%d\n", ++pass, udefcnt);
			changed = 0;
			i = 2;  /* sizeof magic word on disk */
			fpos = i;
			lseek( infile, fpos, 0 );
			while( ((i = read( infile, &ar8k_hd, AR8SIZ)) == AR8SIZ )
			    && ( ar8k_hd.ar8k_name[0] != '\0') ){
				swap_ar(&ar8k_hd);
				if( trysim() ){
					if(debug) fprintf(stderr,"  LOADING member %.8s at fpos=%ld\n",
						ar8k_hd.ar8k_name, fpos);
					if( filtab[ n ].posndx <= 0 )
						filtab[ n ].posndx = posmax+1;
					postab[ ++posmax ] = fpos + AR8SIZ;
					lseek( infile, fpos + AR8SIZ, 0 );
					if( !rdhead() ) return;
					p1load();
					changed = 1;
					if( udefcnt == 0 ) return;
				}
				fpos += AR8SIZ + ar8k_hd.ar8k_size;
				round( fpos );
				lseek( infile, fpos, 0 );
			}
		} while( changed && udefcnt > 0 );
		if( filtab[n].posndx > 0 ) ++posmax;
		return;
	}
	if( symread() == 0 ) return;
	filtab[ n ].posndx = -1;
	lseek( infile, (long)XHSIZ, 0 );
	p1load();
}


void p1load(void)
{
	if( (x_hdr.x_magic & 0xfff0) != X_SU_MAGIC ){
		err("bad magic in %s",fname);
		return;
	}
	if( (x_hdr.x_magic & 0xf) < (X_NU_MAGIC & 0xf ) ){
		if( nonseg ){
			err("%s is a segmented file in a non-segmented load",
				fname);
			return;
		}
		seg++;
	} else {
		if( seg ){
			err("%s is a non-segment file in a segmented load",
				fname);
			return;
		}
		nonseg++;
	}

	p1doseg();
	p1dosym();
}


void p1doseg(void)
{
	reg int i, j, k;
	reg struct x_sg *xp;

	i = x_hdr.x_nseg * sizeof( struct x_sg );
	if( read( infile, x_sg, i ) != i ){
		err("bad segment header read in %s", fname );
		return;
	}
	/* byte-swap all segment headers */
	for( k=0; k < x_hdr.x_nseg; k++ ) swap_sg( &x_sg[k] );

	if( modno >= MODNO ) errex("too many modules at %s",fname);
	modmap[ modno ] = maps = mapx;
	modsym[ modno++ ] = syms = symx;
	mapx += x_hdr.x_nseg;
	symx += (symtop-symlev);
	if( mapx > MAPNO ) errex("map overflow in %s",fname);
	if( symx > SYMPNO ) errex("symap overlow in %s",fname);

	for( i = symlev; i < symtop; i++ ){
		if( symtab[ i ].x_sy_fl == X_SY_SEG ){
			j = lookup( seghash, SEGHASH, symtab[i].x_sy_name );
			if( j != -1 ){
				map[maps+symtab[i].x_sy_sg] = symtab[j].x_sy_sg;
				symap[syms + (i-symlev) ] = j;
			}
		}
	}

	for( xp = x_sg, k = 0; k < x_hdr.x_nseg; xp++, k++ ){
		j = xp->x_sg_no & 0177;
		if( j == 0177 ) j = findseg( xp->x_sg_typ , xp->x_sg_len);
		segtab[j].segtyp = newtyp( segtab[j].segtyp, xp->x_sg_typ);
		map[maps + k ] = j;
		segrel[ k ] = segtab[j].segsiz;
		if( ( segtab[j].segsiz += xp->x_sg_len) >= 65536L )
			err( "segment %d overflow in %s",j,fname);
		round( segtab[j].segsiz );
	}
}


void p1dosym(void)
{
	reg int i, j, k;
	short symstt;
	short discard;
	short *syp;

	symstt = symlev;
	for( i=symlev; i < symtop; i++ ){
		syp = &symap[ syms + (i-symstt) ];
		*syp = symlev;
		j = symtab[i].x_sy_fl;
		switch( j ){

case X_SY_SEG:		k = lookup( seghash, SEGHASH, symtab[i].x_sy_name );
			if( k == -1 ) break;
			*syp = k;
			continue;

case X_SY_LOC:		hpos = &discard;
			if( nolocal ) continue;
			break;

case X_SY_UNX:		k = lookup( symhash, SYMHASH, symtab[i].x_sy_name );
			if( k != -1 ){
				if( symtab[k].x_sy_val < symtab[i].x_sy_val )
					symtab[k].x_sy_val = symtab[i].x_sy_val;
				*syp = k;
				continue;
			}
			udefcnt++;
			break;

case X_SY_GLB:		k = symtab[i].x_sy_sg;
			symtab[i].x_sy_val += segrel[ k ];
			symtab[i].x_sy_sg = map[ maps + k ];
			k = lookup( symhash, SYMHASH, symtab[i].x_sy_name );
			if( k != -1 ){
				*syp = k;
				if( symtab[k].x_sy_fl == X_SY_UNX ){
					udefcnt--;
					symtab[ k ] = symtab[ i ];
				} else {
					err("multiple def: %-8s in %s",
						symtab[i].x_sy_name, fname );
				}
				continue;
			}
		}
		*hpos = symlev;
		symtab[ symlev++ ] = symtab[ i ];
	}
}


int trysim(void)
{
	reg int i, k;

	if( !rdhead() ) return( 0 );
	i = x_hdr.x_magic;
	if( seg && i != X_SU_MAGIC && i != X_SX_MAGIC ) return( 0 );
	if( nonseg && ( i==X_SU_MAGIC || i==X_SX_MAGIC) ) return( 0 );
	if( !symread() ) return( 0 );
	for( i=symlev; i < symtop; i++ ){
		if( symtab[ i ].x_sy_fl == X_SY_GLB ){
			k = lookup( symhash, SYMHASH, symtab[i].x_sy_name );
			if( k != -1 && symtab[k].x_sy_fl == X_SY_UNX )
				return( 1 );
		}
	}
	return( 0 );
}


int symread(void)
{
	reg int i;

	symtop = symlev;
	if( lseek( infile, x_hdr.x_nseg * sizeof( struct x_sg) +
				x_hdr.x_init +
				x_hdr.x_reloc, 1) == -1 ){
		err("symbol seek failed in %s",fname);
		return( 0 );
	}
	symtop = symlev + x_hdr.x_symb/sizeof( struct x_sym );
	if( symtop > SYMNO ) errex("symbol table overflow in %s",fname);
	i = x_hdr.x_symb;
	if( read(infile, &symtab[symlev], i) != i ){
		err( "bad symbol table read in %s", fname );
		return( 0 );
	}
	/* byte-swap all symbols just read */
	for( i = symlev; i < symtop; i++ ) swap_sym( &symtab[i] );
	return( 1 );
}


void interlude(void)
{
	int i, j;

	relvalc = relval = 0;
	if( !nonseg || saverel ) splitid = 0;
	j = 0;
	for( i=0; i<SEGNO; i++ )
		if( segtab[i].segsiz ) j++;
	objnseg = j;
	relpos = XHSIZ + j * sizeof( struct x_sg );
	for( i=0; i<prex; i++ )
		if( segtab[ prelist[i] ].segsiz ) interpos( prelist[i] );
	for( i=0; i<SEGNO; i++ )
		if( segtab[i].segsiz ) interpos( i );
	if( relvalc >= 65536L ) err("code too big for non-seg load");
	if( relval >= 65536L ) err("too big for non-seg load");
	if( nonseg && !saverel )
		for( i=1; i<symlev; i++ )
			if( symtab[i].x_sy_fl & (X_SY_GLB & X_SY_LOC) )
				symtab[i].x_sy_val +=
					segtab[ symtab[i].x_sy_sg ].segrel;

	if( debug ) segdmp("end of interlude");
}


void interpos(int i)
{
	if( idata[ segtab[i].segtyp] ){
		segtab[i].segpos = relpos;
		relpos += segtab[i].segsiz;
	}
	if( splitid && segtab[i].segtyp == X_SG_COD ){
		segtab[i].segrel = relvalc;
		relvalc += segtab[i].segsiz;
	} else {
		segtab[i].segrel = relval;
		if( nonseg && !saverel )
			relval += segtab[i].segsiz;
	}
	segtab[i].hiwat = segtab[i].segsiz;
	segtab[i].segsiz = 0;
}


void loadmap(void)
{
	reg int i;

	fprintf( stdout, "load map for %s\n\nsegment sizes:\n\n",objname );
	for( i=0; i<SEGNO; i++ )
		if( segtab[i].hiwat )
			fprintf( stdout,
				"seg[%d]  %d  %8ld  %8ld\n",
				i,
				segtab[i].segtyp,
				(long)segtab[i].hiwat,
				segtab[i].segpos);

	fprintf( stdout, "\nsymbols:\n\n" );
	for( i=1; i < symlev; i++ )
		fprintf( stdout, "%3d %-8s  %3d  %d  %4x\n",
			i,
			symtab[i].x_sy_name,
			symtab[i].x_sy_sg & 0377,
			symtab[i].x_sy_fl,
			symtab[i].x_sy_val );
}


void pass2(void)
{
	reg int i;

	modno = 0;
	for( i=0; i < filndx; i++ ){
		fname = filtab[ i ].filname;
		if( filtab[ i ].posndx ){
			if( fname[0] == '-' ){
				ap = libname;
				append( "lib" );
				append( fname+2 );
				append( ".a" );
				infile = openb( libname, 0 );
				if( infile < 0 ){
					errex("p2 can't open %s",fname);
				}
			} else {
				infile = openb( fname , 0 );
				if( infile < 0 )
					errex( "p2 can't open %s", fname );
			}
			p2read( i );
			close( infile );
		}
	}
}


void p2read(int n)
{
	reg int i;

	i = filtab[ n ].posndx;
	if( i == 0 ) return;
	if( i > 0 ){
		while( postab[ i ] ){
			lseek( infile, postab[ i++ ], 0 );
			p2load();
		}
		return;
	}
	p2load();
}


void p2load(void)
{
	reg int i, j;
	reg struct x_sg *xp;
	reg uns l;
	int k;

	if( !rdhead() )return;
	i = x_hdr.x_nseg * sizeof( struct x_sg );
	if( read( infile, x_sg, i ) != i )
		errex("bad seg read in p2 %s", fname );
	/* byte-swap all segment headers */
	for( k=0; k < x_hdr.x_nseg; k++ ) swap_sg( &x_sg[k] );

	maps = modmap[ modno ];
	syms = modsym[ modno++ ];
	for( xp=x_sg, i=0; i < x_hdr.x_nseg; xp++, i++ ){
		j = map[ maps + i ];
		segsiz[ i ] = segtab[ j ].segsiz;
		segrel[ i ] = segtab[ j ].segrel + segtab[ j ].segsiz;
		segpos[ i ] = segtab[ j ].segpos;
		segtab[ j ].segsiz += xp->x_sg_len;
		round( segtab[ j ].segsiz );
		if( idata[ xp->x_sg_typ ] ){
			uns inlen;

			lseek( objfile, segpos[ i ], 0 );
			inlen = xp->x_sg_len;
			l = inlen;
			round( l );
			segtab[ j ].segpos += l;
			bpos = 1;
			while( inlen ){
				k = inlen > 512 ? 512 : inlen;
				if( read( infile, buf , k ) != k)
					errex("bad read in %s",fname);
				if( write( objfile, buf, k ) != k)
					errex("bad write in %s",objname);
				inlen -= k;
			}
		}
	}
	p2dorel();
}


void p2dorel(void)
{
	long l;
	reg int i;
	reg struct x_rel *rlp;

	l = x_hdr.x_reloc;
	while( l ){
		i = l > RSIZ ? RSIZ : l;
		if( read( infile, rlibuf, i ) != i )
			errex("bad relocation read in %s",fname);
		l -= i;
		/* byte-swap relocation entries */
		{
			int n, cnt = i / sizeof(struct x_rel);
			for( n=0; n < cnt; n++ ) swap_rel( &rlibuf[n] );
		}
		rlp = rlibuf;
		while( i > 0 ){
			reloc( rlp );
			i -= sizeof( struct x_rel );
			rlp++;
		}
	}
	flshbuf();
}


void reloc(struct x_rel *rlp)
{
	reg int j;
	int s;  /* segment number */
	uns val;
	long pos;
	int off;
	uint16_t tmp;

	if( rlp->x_rl_flg & 04 ){  /* external item in symbol table */
		j = symap[ syms + rlp->x_rl_bas ];
		s = 127;
		if( saverel ){
			val = 0;
			rlp->x_rl_bas = j-1;
		} else {
			val = symtab[ j ].x_sy_val;
			j = symtab[ j ].x_sy_sg & 0177;
			if( j != 127 ) s = map[ maps + j ];
		}
	} else {
		j = rlp->x_rl_bas;
		s = map[ maps + j ];
		val = segrel[ j ];
		if( saverel ) rlp->x_rl_bas = s;
	}
	pos = segpos[ rlp->x_rl_sgn ] + rlp->x_rl_loc;
	if( saverel ){
		rlp->x_rl_loc += segsiz[ rlp->x_rl_sgn ];
		rlp->x_rl_sgn = map[maps+rlp->x_rl_sgn];
		putrel( rlp );
	}
	off = pos & 511;
	getbuf( pos );
	dirty++;
	switch( rlp->x_rl_flg & 03 ){

case X_RL_OFF:	/* relocate a 16 bit simple offset */
		tmp = rd16be(&buf[off]);
		tmp += val;
		wr16be(&buf[off], tmp);
		break;

case X_RL_SSG:	/* relocate a short seg plus offset */
		tmp = rd16be(&buf[off]);
		val += tmp & 0377;
		if( val > 255 ) err("short segment offset overflow in %s",fname);
		wr16be(&buf[off], ( s << 8 ) | val);
		break;

case X_RL_LSG:	/* relocate a long seg plus offset */
		wr16be(&buf[off], 0100000 | (s << 8));
		pos += 2;
		getbuf( pos );
		off = pos & 511;
		tmp = rd16be(&buf[off]);
		tmp += val;
		wr16be(&buf[off], tmp);
		dirty++;
	}
}


void finale(void)
{
	reg int i, j;
	struct x_hdr out_hdr;

	x_hdr.x_magic = (udefcnt == 0);
	x_hdr.x_magic |= nonseg ?
				(splitid ? X_NUI_MAGIC : X_NU_MAGIC) :
				X_SU_MAGIC;
	x_hdr.x_nseg = objnseg;
	x_hdr.x_init = relpos - XHSIZ - objnseg * sizeof( struct x_sg );
	x_hdr.x_reloc = 0;
	x_hdr.x_symb = strip && !saverel ? 0 : (symlev-1)*sizeof(struct x_sym);

	if(debug)segdmp( "finale" );

	/* write the segment descriptors */
	lseek( objfile, (long)XHSIZ, 0 );
	for( j=0; j<prex; j++ ) finout( prelist[j] );
	for( i=0; i<SEGNO; i++ ) finout( i );

	/* write the relocation data */
	lseek( objfile, relpos, 0 );
	if( saverel ){
		wrrel();
		lseek( relfile, 0L, 0 );
		while( (i = read( relfile, rlibuf, RSIZ )) > 0 ){
			x_hdr.x_reloc += i;
			write( objfile, rlibuf, i );
		}
	}

	/* write the symbol table (swap to big-endian) */
	for( i=1; i<symlev; i++ ) swap_sym( &symtab[i] );
	write( objfile, &symtab[1], (int)x_hdr.x_symb );

	/* write the header last (swap to big-endian) */
	out_hdr = x_hdr;
	swap_hdr( &out_hdr );
	lseek( objfile, 0L, 0 );
	write( objfile, &out_hdr, XHSIZ );
	close( objfile );
}


void finout(int i)
{
	struct x_sg tmp;

	if( segtab[i].hiwat ){
		tmp.x_sg_no = saverel ? 255 : i;
		tmp.x_sg_typ = segtab[i].segtyp;
		tmp.x_sg_len = segtab[i].hiwat;
		swap_sg( &tmp );
		write( objfile, &tmp, sizeof( struct x_sg ));
		segtab[i].hiwat = 0;
	}
}


void dodline(char *s)
{
	reg char *cp;
	reg int i, j, k;
	int ch;
	struct x_sym *syp;
	char *ssave;

	ssave = s;
	j = i = 0;
	if( isalpha( *s ) ){
		cp = idbuf;
		while( isalpha( *s ) ){
			i = *s++;
			if( cp < &idbuf[XNAMELN] ) *cp++ = i;
		}
		while( cp < &idbuf[XNAMELN] ) *cp++ = 0;
if(debug)fprintf(stderr,"idbuf = %s\n",idbuf);
		if( idmatch( idbuf, "text" ) ) j = X_SG_COD; else
		if( idmatch( idbuf, "data" ) ) j = X_SG_DAT; else
		if( idmatch( idbuf, "bss" ) ) j = X_SG_BSS; else
		if( idmatch( idbuf, "cons" ) ) j = X_SG_CON; else
			goto illdesc;
		if( *s != '=' ) goto illdesc;
		ch = ',';
		while( ch == ',' ){
			s++;
			if( !isdigit( *s ) ) goto illdesc;
			i = 0;
			while( isdigit( *s ) ) i = i*10 + *s++ - '0';
			ch = *s;
			k = i;
			if( ch == '-' ){
				s++;
				if( !isdigit( *s ) ) goto illdesc;
				k = 0;
				while( isdigit( *s ) ) k = k*10 + *s++ - '0';
				ch = *s;
			}
			if( i < 0 || i > SEGNO || k < 0 || k > SEGNO )
					goto illdesc;
			for(;;){
				dodset( i, j );
				if( i == k ) break;
				if( i < k ) i++; else i--;
			}
		}
		if( ch != 0 ) goto illdesc;
		return;
	}
	if( !isdigit( *s ) ) goto illdesc;
	while( isdigit( *s ) ) i = i*10 + *s++ - '0';
	if( i < 0 || i >= SEGNO )goto illdesc;
	j = 0;
	while( *s == ',' ){
		s++;
		switch( *s ){
	case 0:		goto illdesc;
	case 'T':
	case 't':	j = X_SG_COD;  break;
	case 'D':
	case 'd':	j = X_SG_DAT;  break;
	case 'C':
	case 'c':	j = X_SG_CON;  break;
	case 'B':
	case 'b':	j = X_SG_BSS;  break;
	default:	goto illdesc;
		}
		s++;
		dodset( i , j );
		j = 0;
	}

	dodset( i , j );
	if( *s == 0 ) return;
	if( *s != '=' )goto illdesc;
	ch = ',';
	while( ch == ',' ){
		s++;
		cp = idbuf;
		while( *s && *s != ',' ) *cp++ =  *s++;
		while( cp < &idbuf[XNAMELN] ) *cp++ = 0;
		ch = *s;
		j = lookup( seghash , SEGHASH, idbuf);
		if( j == -1 ) *hpos = j = install( idbuf );
		syp = &symtab[ j ];
		j = syp->x_sy_sg & 0177;
		if( j != 127 && j != i ){
			err( "multiple definition of segment: %s" , idbuf );
			goto illdesc;
		}
		syp->x_sy_sg = i;
		syp->x_sy_fl = X_SY_SEG;
		j = 0;
		if( idmatch( idbuf, "_text" ) ) j = X_SG_COD; else
		if( idmatch( idbuf, "_data" ) ) j = X_SG_DAT; else
		if( idmatch( idbuf, "_bss" ) ) j = X_SG_BSS; else
		if( idmatch( idbuf, "_cons" ) ) j = X_SG_CON;
		if( j ) segtab[i].segtyp = newtyp( segtab[i].segtyp, j );
	}
	if( ch == 0 ) return;

illdesc:
	err( "descriptor format" );
	fprintf(stderr,"\t%s\n\t",ssave);
	j = cp - line;
	while( --j >= 0 ) putc( ' ' , stderr );
	putc( '|', stderr );
	putc( '\n', stderr );
}


void dodset(int i, int typ)
{
	if( !segtab[i].segalloc ){
		prelist[ prex++ ] = i;
		segtab[i].segalloc = 1;
	}
	if( typ ) segtab[i].segtyp = newtyp(segtab[i].segtyp, typ );
}



void dodfile(char *s)
{
	reg char *cp;
	reg int i;

	/* Replace CP/M opena: close stdin and reopen as the descriptor file */
	if( freopen( s, "r", stdin ) == NULL ){
		err( "can't open descriptor file: %s", s );
		return;
	}
	cp = line;
	while( (i = getchar()) != EOF ){
		if( i != ' ' ){
			if( i == '\n' ){
				*cp = 0;
				dodline( cp = line );
			} else	*cp++ = i;
		}
	}
	if( debug ){
		fprintf(stderr,"preallocation list:\n" );
		for( i=0; i<prex; i++ )
			fprintf(stderr,"prelist[%d] = %d\n",i,prelist[i]);
	}
}


void append(char *s)
{
	while( (*ap++ = *s++) );
	ap--;
}


void err(const char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	fprintf( stderr , "error: " );
	vfprintf( stderr , s, ap );
	fprintf( stderr , "\n" );
	va_end(ap);
	errcnt++;
}

void errex(const char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	fprintf( stderr , "fatal error: " );
	vfprintf( stderr , s, ap );
	fprintf( stderr , "\n" );
	va_end(ap);
	exit( 1 );
}


int findseg(int typ, uns siz)
{
	reg int i, j;

	for( j=0; j<prex; j++ ){
		i = prelist[ j ];
		if(	oktyp( segtab[i].segtyp, typ ) &&
			segtab[i].segsiz + siz < 65536L )
				return( i );
	}
	for( i=0; i<SEGNO; i++ )
		if(	oktyp( segtab[i].segtyp, typ) &&
			!segtab[i].segalloc &&
			segtab[i].segsiz + siz < 65536L )
				return( i );
	errex("segment overflow in %s",fname);
	return 0; /* not reached */
}


void flshbuf(void)
{
	if( dirty ){
		lseek( objfile, bpos, 0 );
		write( objfile, buf, blen );
		dirty = 0;
		bpos = 1;
	}
}


void getbuf(long p)
{
	p &= -512;
	if( bpos != p ){
		if( dirty ) flshbuf();
		bpos = p;
		lseek( objifile, bpos, 0 );
		blen = read( objifile, buf, 512 );
	}
}


int idmatch(char *a, char *b)
{
	reg int i, j;

	for( i=0; i<XNAMELN; i++ ){
		j = *a++;
		if( j == 0 ) return( *b == 0 );
		if( j != *b++ ) return( 0 );
	}
	return( 1 );
}


int install(char *s)
{
	reg char *cp;
	int i;

	i = symlev;
	if( i >= SYMNO ) errex( "symbol table overflow" );
	symtab[ i ].x_sy_sg = 255;
	symtab[ i ].x_sy_fl = X_SY_UNX;
	cp = symtab[ i ].x_sy_name;
	while( (*cp++ = *s++) );
	symlev++;
	return( i );
}


int lookup(short *htab, int HTAB, char *sym)
{
	reg int i, j;
	reg char *cp;
	int k;

	i = 0;
	cp = sym;
	for(k = 0; k < XNAMELN && *cp; k++) i = i*5 + *cp++;
	i &= HTAB-1;
	for( k=0; k<2; k++ ){
		while( i < HTAB ){
			if( ( j = htab[ i ]) ){
				if( idmatch( sym, symtab[ j ].x_sy_name ))
					return( j );
			} else {
				hpos = &htab[ i ];
				return( -1 );
			}
			i++;
		}
		i = 0;
	}
	errex( "hash table overflow: %d", HTAB );
	return -1; /* not reached */
}


int mk_outfile(char *s)
{
	int i;

	close( creatb( s , 0666 ) );
	i = openb( s , 2 );
	return( i );
}


void putrel(struct x_rel *rlp)
{
	if( rlo >= RLC ) wrrel();
	rlobuf[ rlo++ ] = *rlp;
}


int rdhead(void)
{
	if( read( infile, &x_hdr, XHSIZ ) == XHSIZ ){
		swap_hdr( &x_hdr );
		if(debug>2){
			fprintf(stderr,
				"%s: magic = %x nseg = %d init = %ld reloc = %ld symb = %ld\n",
				fname, (unsigned)x_hdr.x_magic, x_hdr.x_nseg,
				(long)x_hdr.x_init, (long)x_hdr.x_reloc, (long)x_hdr.x_symb);
		}
		return( 1 );
	}
	err( "unexpected EOF while reading header on %s", fname );
	return( 0 );
}


void setstack(char *s)
{
	long l = 0;

	while( *s >= '0' && *s <= '9' )
		l = l * 10 + ( *s++ - '0' );
	if( l & 1 ) l++;
	if( l < 0 || l > 65536 ){
		err( "stack segment too big" );
		return;
	}
	round( l );
	stacksiz = l;
}


void wrrel(void)
{
	int i;

	if( rlo ){
		/* swap to big-endian before writing */
		for( i=0; i<rlo; i++ ) swap_rel( &rlobuf[i] );
		write( relfile, rlobuf, rlo * sizeof( struct x_rel ) );
		rlo = 0;
	}
}



void segdmp(char *s)
{
	reg int i;

	fprintf( stderr,"segdmp called from %s\n" ,s);
	fprintf( stderr,"old:new typ siz hiwat segrel pos\n");
	for( i=0; i<SEGNO; i++ )
		if( segtab[i].segsiz || segtab[i].hiwat || segtab[i].segpos )
			segsho( 0 , i );
}

void segsho(char *s, int i)
{
	if( s ) fprintf( stderr , "%s", s );
	fprintf( stderr,
		"seg[%d]:%d %d  %ld  %ld  %ld  %ld\n",
		i,
		segtab[i].segalloc,
		segtab[i].segtyp,
		segtab[i].segsiz,
		(long)segtab[i].hiwat,
		(long)segtab[i].segrel,
		segtab[i].segpos);
}
