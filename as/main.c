/*
    Copyright 1983
    Alcyon Corporation
    8716 Production Ave.
    San Diego, Ca.  92121
*/

/*
 * a two pass relocatable assembler for the Motorola 68000 microprocessor
 *
 *  Bill Allen
 *  Modified by Vicki Hutchison
 */

#include "as68.h"
#include "def.h"
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifndef __ALCYON__
int mkstemp(char *template);
#define mktemp(f) close(mkstemp(f))
#endif


short mode;             /* operand mode (byte, word, long) */
short modelen;          /* operand length per mode */

short fchr;             /* first char in term */
FILE *ifn;              /* source file descriptor */
int pitix;              /* ptr to it buffer */
short itwc;             /* number of words in it buffer */
struct it *pitw;        /* ptr to it buffer next entry */
short itype;            /* type of item */
union iival ival;       /* value of item */
struct symtab *lblpt;   /* label pointer */
char lbt[SYEXTNAMLEN + 1]; /* holds label name */
int32_t loctr;          /* location counter */
int32_t savelc[4];      /* save relocation counters for 3 bases */
short nite;             /* number of entries in stbuf */
struct it *pnite;
struct symtab *opcpt;   /* pointer to opcode entry in main table */
short p2flg;            /* 0=>pass 1  1=>pass 2 */
struct irts *pirt;      /* entry in initial reference table */
short reloc;            /* reloc value returned by expr evaluator (expr) */
short rlflg;            /* relocation value of current location counter */
struct hdr2 couthd;     /* cout header structure */

short format;
FILE *itfn;             /* it file number */
short prtflg;           /* print output flag */
short undflg;           /* make undefined symbols external flag */

short starmul;          /* * is multiply operator */

struct symtab *endptr;
struct symtab *addptr;
struct symtab *orgptr;
struct symtab *subptr;
struct symtab *addiptr;
struct symtab *addqptr;
struct symtab *subiptr;
struct symtab *subqptr;
struct symtab *cmpptr;
struct symtab *addaptr;
struct symtab *cmpaptr;
struct symtab *subaptr;
struct symtab *cmpmptr;
struct symtab *dcptr;
struct symtab *andptr;
struct symtab *andiptr;
struct symtab *eorptr;
struct symtab *eoriptr;
struct symtab *orptr;
struct symtab *oriptr;
struct symtab *cmpiptr;
struct symtab *moveptr;
struct symtab *moveqptr;
struct symtab *exgptr;
struct symtab *evenptr;
struct symtab *jsrptr;
struct symtab *bsrptr;
struct symtab *nopptr;

short plevel;           /* parenthesis level counter */
short opdix;            /* operand index counter */

short *pins;
short *prlb;
short ins[5];           /* holds instruction words */

short extflg, extref;   /* external in expr */

struct op opnd[2];

FILE *lfil;				/* loader output file descriptor */
FILE *dafil;            /* temp file for data stuff */
FILE *trfil;			/* temp for text relocation bits */
FILE *drfil;            /* temp for data relocation bits */

char itfilnam[PATH_MAX];
char dafilnam[PATH_MAX];
char trfilnam[PATH_MAX];
char drfilnam[PATH_MAX];
char ldfn[PATH_MAX];        /* name of the relocatable object file */
char *sfname;				/* Source filename */

/* assembler flag variables */
short didorg;
short shortadr;         /* short addresses if set */
short m68010;           /* 68010 code */
short aesflag;
char *lineftbl;

/* pass 1 global variables */
short numops;           /* number of operands */
short inoffset;         /* offset directive */
short p1inlen;          /* pass 1 instr length */

/* pass 2 global variables */
short instrlen;         /* pass 2 bytes in current instruction */
  
/* General Assembler Variables */
char peekc;
short ca_true;          /* true unless in a false CA */
short ca;               /* depth of conditional assembly, none = 0 */
short ca_level;         /* at what CA depth did CA go false? */
short nerror;           /* # of assembler errors */
short in_err;           /* don't generate instrlen err if in err state */
int32_t itoffset;
short equflg;           /* doing an equate stmt */
short refpc;            /* * referenced in expr */


/*
 * Define Temporary and Init file names per O/S.  We use code here to
 * allow re-directing temp files and the init file via command line
 * switches.
 */

static const char *tdname = "";				/*  Temp files in same directory    */
static char const tfilebase[] = "a6XXXXXX";		/*  Temp file  basename         */

static int symcon;
static int explmode;   /* explicit mode length given */
static int opcval;     /* opcode */
static int chmvq;

short indir[2];
short immed[2];
static short numcon[2];
static short numsym[2];
static short numreg[2];


/* get a temp file for the intermediate text */
static FILE *gettempf(P(char *) filnam)
PP(char *filnam;)
{
	FILE *fp;
	
	/* Build temp file names */
	strcpy(filnam, tdname);
	strcat(filnam, tfilebase);
	/* Make it unique */
	mktemp(filnam);
	
	fp = fopen(filnam, "w+b");
	if (fp != NULL)
		return fp;
	rpterr(_("temp file create error: %s %s"), filnam, strerror(errno));
	endit();
}


static VOID rubout(P(int) sig)
PP(int sig;)
{
	UNUSED(sig);
	nerror = -1;
	endit();
}


static VOID mystrncpy(P(char *) astr1, P(const char *) astr2, P(int) alen)
PP(char *astr1;)
PP(const char *astr2;)
PP(register int alen;)
{
	register char *str1;
	register const char *str2;

	str1 = astr1;
	str2 = astr2;
	while (--alen >= 0)
		*str1++ = *str2++;
}


static VOID as_getmode(NOTHING)
{
	if (fchr == '.')
	{
		fchr = gchr();
		switch (fchr)
		{
		case 'b':
		case 'B':
		case 's':
		case 'S':
			modelen = BYTESIZ;
			mode = BYTE;
			break;
		case 'w':
		case 'W':
			modelen = WORDSIZ;
			mode = WORD;
			break;
		case 'l':
		case 'L':
			modelen = LONGSIZ;
			mode = LONG;
			break;
		default:
			peekc = fchr;
			fchr = '.';
			goto getm1;
		}
		explmode++;
		fchr = gchr();
		igblk();
		return;
	}
  getm1:
	if (opcpt == exgptr)
	{									/* length is long */
		modelen = LONGSIZ;
		mode = LONG;
	} else
	{
		mode = WORD;					/* default is word */
		modelen = WORDSIZ;
	}
}


/* check to be sure specified mode is legal */
static int modeok(NOTHING)
{
	switch (format)
	{
	case FORMAT_NONE:
	case FORMAT_STOP:
	case FORMAT_TRAP:
		return FALSE;
	case FORMAT_EXT:
	case FORMAT_ADDA:
	case FORMAT_MOVEM:
	case FORMAT_MOVEP:
		return modelen == BYTESIZ ? FALSE : TRUE;
	case FORMAT_ABCD:
	case FORMAT_SCC:
		return modelen == BYTESIZ ? TRUE : FALSE;
	case FORMAT_BTST:
	case FORMAT_JMP:
		return modelen == WORDSIZ ? FALSE : TRUE;
	case FORMAT_DIV:
	case FORMAT_DBCC:
	case FORMAT_SWAP:
		return modelen == WORDSIZ ? TRUE : FALSE;
	case FORMAT_RELBR:
		return TRUE; /* now also valid for bra.l */
	case FORMAT_EXG:
	case FORMAT_LEA:
	case FORMAT_MOVEQ:
	case FORMAT_PEA:
		return modelen == LONGSIZ ? TRUE : FALSE;
	default:
		return TRUE;
	}
}


static int shiftea(P(int) lidx)
PP(int lidx;)
{
	if (indir[lidx] && numreg[lidx])
	{
		if (numreg[lidx] == WORD_ID)
		{
			numcon[lidx] = 1;
			return WORDSIZ;
		}
		if (numreg[lidx] == LONG_ID)
		{
			numcon[lidx] = 2;
			return LONGSIZ;
		}
		return (numcon[lidx] || numsym[lidx]) ? WORDSIZ : 0;
	}
	if (numsym[lidx])
	{
		if (numreg[lidx] == WORD_ID)
		{
			numcon[lidx] = 1;
			return WORDSIZ;
		}
		if (numreg[lidx] == LONG_ID)
		{
			numcon[lidx] = 2;
			return LONGSIZ;
		}
		return LONGSIZ;
	}
	if (numcon[lidx])
		return numcon[lidx] == 2 ? LONGSIZ : WORDSIZ;
	return 0;
}


/* calc the length of an effective address */
static int lenea(P(int) lidx)
PP(int lidx;)
{
	int l;
	
	if (immed[lidx])
		l = mode == LONG ? LONGSIZ : WORDSIZ;
	else
		l = shiftea(lidx);
	return l;
}


/* calculate the instruction length in bytes */
static int calcilen(NOTHING)
{
	register short i;
	register long l;
	register struct symtab *p;

	i = 2;								/* all instrs at least 2 bytes */

	switch (format)
	{
	case FORMAT_MOVEM:
		i += 2;							/* for reg mask */
		/* fall through */
	case FORMAT_2EA:					/* two ea's -- one of which may be a reg */
	case FORMAT_ADDA:
	case FORMAT_LEA:
	case FORMAT_CMP:
	case FORMAT_DIV:
	case FORMAT_MOVE:
	case FORMAT_MOVEP:
		i += lenea(1);
		/* fall through */
	case FORMAT_INC:
	case FORMAT_CLR:
	case FORMAT_SCC:
	case FORMAT_PEA:
		i += lenea(0);
		break;

	case FORMAT_JMP:					/* explicit jmp length... */
		if (!explmode)
			i += lenea(0);
		else
			return mode == LONG ? 6 : 4;	/* explicit jmp.? */
		break;

	case FORMAT_BTST:
		i += immed[0] ? 2 + lenea(1) : lenea(1);
		break;

	case FORMAT_STOP:
	case FORMAT_DBCC:
	case FORMAT_LINK:
	case FORMAT_MOVEC:
		i += 2;							/* always 4 bytes */
		break;

	case FORMAT_RELBR:					/* relative branches */
		if (itwc == ITOP1 + 1)
		{
			if (stbuf[ITOP1].itty == ITCN || stbuf[ITOP1].itty == ITCW)
			{
				l = stbuf[ITOP1].itop.l;
			} else if (stbuf[ITOP1].itty == ITSY)
			{
				p = stbuf[ITOP1].itop.ptrw2;
				if (p->flags & SYDF)
					l = p->vl1;			/* symbol value */
				else
					goto loffst;
			} else
			{
				goto loffst;
			}
			l -= (loctr + 2);
			if ((!explmode || modelen == BYTESIZ) && l <= 127 && l >= -128)	/* 8 bit offset */
				break;
		}
	  loffst:
		if (!explmode || modelen > BYTESIZ)	/* recognize br extensions */
		{
			i += 2;						/* long offset for branches */
			if (modelen >= LONGSIZ)
				i += 2;
		}
		break;

	case FORMAT_ADDI:
		i += (mode == LONG ? LONGSIZ : WORDSIZ) + lenea(1);
		break;

	case FORMAT_EOR:
		if (immed[0])
			i += (mode == LONG ? LONGSIZ : WORDSIZ);
		/* fall through */
	case FORMAT_ADDQ:
	case FORMAT_MOVEQ:
		i += lenea(1);
		break;

	case FORMAT_ASL:
		if (numops == 1)				/* memory shift instruction */
			i += shiftea(0);
		break;
	}

	return i;
}


#define NOCODE ((i >= 0 && i < 6) || i == 9 || i == 11 || i == 12 || i == 16 || (i >= 20 && i <= 30))
/* cond-directives, section, ds, set, equ, reg, globl, end, offset */

/*
 * create intermediate text (it) for one statement
 * call with first character of statement in fchr
 */
static VOID cisit(NOTHING)
{
	register char *p1, *p2;
	register adirect dirop;
	register short i, col1;		/* col1 labels in col 1... */
	char str[SYEXTNAMLEN], *l;
	char tlab1[SYEXTNAMLEN];

  ciss1:
	immed[0] = immed[1] =
	indir[0] = indir[1] =
	numcon[0] = numcon[1] =
	numsym[0] = numsym[1] =
	numreg[0] = numreg[1] = 0;
	plevel = numops = opdix = explmode = 0;
  cistop:
	col1 = 1;
	if (fchr == EOLC)
	{
		fchr = gchr();
		goto cistop;
	}
	if (fchr == ' ')
	{
		col1 = 0;
		igblk();
		if (fchr == EOLC)				/* blank line */
			goto cistop;
		peekc = fchr;
		if (fchr != CEOF)
			fchr = ' ';					/* catch eof... */
	}
	if (fchr == CEOF)
		return;
	if (fchr == '*')
	{									/* ignore comments */
		fchr = gchr();
		if (fchr == '=')
		{								/* relocation counter assignment */
			fchr = gchr();				/* pass the = */
			horg();						/* output constants if not bss */
		}
		igrst();
		fcflg = 0;						/* clear expr first time flag for next stmt */
		goto ciss1;
	}

	/* get the opcode and label */

	mode = 'w';							/* word mode */
	igblk();							/* ignore blanks */
	poslab = 1;
	gterm(TRUE);
	poslab = 0;
	if (fchr == ':' || fchr == '=')
	{									/* there is a label */
	  label:
		col1 = 0;
		if (itype != ITSY)
		{								/* not a symbol */
			uerr(2);					/* invalid label */
			lbt[0] = 0;
		} else
		{
			p2 = lmte->name;
			for (p1 = lbt; p1 < &lbt[SYEXTNAMLEN];)
			{
				*p1++ = *p2++;
			}
			if (fchr == ':')
				fchr = gchr();			/* ignore the colons */
		}
	  labl1:
		ligblk();
		if (fchr == CEOF)
			return;
		if (fchr == '*')
		{
			igrst();					/* comment */
			goto labl1;
		}
		gterm(TRUE);
		if (fchr == ':' || fchr == '=')
		{								/* another label */
			if (lbt[0])
			{
				mystrncpy(tlab1, lmte->name, SYEXTNAMLEN);	/* save current label */
				dlabl();				/* define the last one */
				pack(tlab1, lmte);		/* restore the old label */
			}
			goto label;
		}
	} else
	{
		lbt[0] = 0;						/* no label */
	}
	
	igblk();
	if (fchr == '=')
		goto label;
	if (itype == ITSP)
	{
		if (ival.oper == '=')
		{
			hequ();
			return;
		}
	}
	if (itype != ITSY)					/* not valid opcode */
		goto cisi3;
	if (col1)
	{									/* could be a label save as is... */
		l = str;
		mystrncpy(l, lmte->name, SYEXTNAMLEN);
	}
	if ((opcpt = lemt(TRUE, oirt)) == lmte)
	{									/* not in opcode table */
		if (col1)
		{								/* it's a label... */
			mystrncpy(lmte->name, l, SYEXTNAMLEN);
			goto label;
		}
	  cisi3:
		if (ca_true)					/* report error if not in CA false */
			xerr(3); /* invalid opcode */
		igrst();
		return;
	}
	as_getmode();							/* look for .b .w or .l mode flag */
	if (opcpt->flags & OPDR)
	{									/* its a directive */
		i = opcpt->vl1;
		if (!ca_true && (i < LOW_CA || i > HI_CA))
		{
			igrst();
			return;
		}
		if (inoffset)
		{
			if (!(NOCODE))
			{
				xerr(12); /* no code or data allowed in offset */
				return;
			}
		}
		dirop = p1direct[i];			/* call routine to handle directive */
		(*dirop) ();
		return;
	} else if (!ca_true)
	{
		igrst();
		return;
	} else if (inoffset)
	{
		xerr(12); /* no code or data allowed in offset */
		return;
	}

	opcval = opcpt->vl1;				/* opcode */
	format = opcpt->flags & OPFF;		/* format of this instr */
	if (explmode)
	{
		if (!modeok())
		{
			xerr(16); /* illegal extension */
			return;
		}
		if (format == FORMAT_RELBR && modelen == LONGSIZ)
		{
			uerr(46); /* warning: bra.l generated for 68020+" */
		}
	}
	dlabl();							/* define label */
	opitb();							/* beginning of statement */
	if (format != FORMAT_NONE)
		opito();						/* may have operands */
	else
		igrst();						/* only comments */
	format = (opcpt->flags & OPFF);		/* may have changed */


	/* end of statement */

	i = calcilen();
	stbuf[1].itrl = i;					/* assumed instruction length */
	stbuf[0].itrl = itwc;				/* number of it entries */
	wostb();							/* write out statement buffer */
	loctr += i;
}


/* main loop */
static VOID mloop(NOTHING)
{
	while (fchr != CEOF)
	{
		fcflg = 0;						/* first time thru expr pass one */
		cisit();						/* create it for one statement */
	}
	opcpt = endptr;
	hend();
}


/*
 * define a label if there is one to define
 *  call with:
 *      label name in lbt if it exists
 *      else lbt[0] == 0
 */
VOID dlabl(NOTHING)
{
	if (lbt[0])
	{									/* got a label */
		pack(lbt, lmte);				/* put label in main table */
		lblpt = lemt(FALSE, sirt);		/* look up label */
		if (lblpt != lmte)
		{								/* symbol entered previously */
			if (lbt[0] == '~')
			{							/* local symbol -- may be duplicate */
				lblpt = lmte;
				mmte();
			} else
			{
				if (lblpt->flags & SYXR)
				{
					uerr(29); /* bad use of symbol */
					lblpt = NULL;
					return;
				}
				if (lblpt->flags & SYDF)
				{
					uerr(1, lblpt->name);	/* label redefined */
					lblpt = NULL;
					return;
				}
			}
		} else
		{
			mmte();						/* make label entry in main table */
		}
		lblpt->flags |= SYDF;			/* label is now defined */
		if (rlflg == TEXT)
			lblpt->flags |= SYRO;
		else if (rlflg == DATA)
			lblpt->flags |= SYRA;
		else if (rlflg == BSS)
			lblpt->flags |= SYBS;
		/* No flags to set if absolute */
		lblpt->vl1 = loctr;				/* label value */
	} else
	{
		lblpt = NULL;
	}
}


/* change clr.l An to suba.l An,An */
static VOID chgclr(NOTHING)
{
	register struct symtab *p;

	if (itype == ITSY)
	{									/* first op is symbol */
		p = lemt(FALSE, sirt);
		if (p == lmte)
			return;
		if (!(p->flags & SYER) || p->vl1 < AREGLO)	/* not A reg */
			return;
		opcpt = subaptr;				/* make it a suba instr */
		opitb();
		opitoo();						/* output first operand -- An */
		itype = ITSP;
		ival.l = ',';
		opitoo();						/* output a comma */
		itype = ITSY;					/* now the A reg again */
	}
}


/*
 * output it for operands
 *      gets intput from gterm
 *      puts output in stbuf using itwc as an index
 *      itwc should point at the next entry to be made in stbuf
 */
VOID opito(NOTHING)
{
	register int lopcomma;

	lopcomma = symcon = chmvq = 0;
	numops++;							/* count first operand */
	for (;;)
	{
		starmul = symcon;				/* star is multiply op if flag is set */
		if (fchr == '\'' || fchr == '"')
			lopcomma = 0;
		gterm(FALSE);					/* get a term */
		if (itwc == ITOP1 && format == FORMAT_CLR && opcval == CLRVAL)
			chgclr();
		opitoo();						/* output it for one operand */
		if (itype == ITSP && ival.oper == ',')
		{
			if (plevel == 1 && !numcon[opdix])
				numcon[opdix] = 1;
			if (lopcomma)
				uerr(30);	/* invalid data list */
			lopcomma++;
			igblk();					/* ignore blanks for 68000 C compiler */
		} else
		{
			lopcomma = 0;
		}
		if (ival.oper == EOLC && itype == ITSP)	/* end of operands */
			break;
		if (fchr == EOLC)
		{
			fchr = gchr();
			break;
		}
	}
	if (chmvq)							/* changed move to moveq */
	{
		if (numops != 2 || immed[1] || indir[1] || numcon[1] || numsym[1] || numreg[1] >= AREGLO)
		{
			stbuf[2].itop.ptrw2 = opcpt = moveptr;	/* change it back */
		}
	}
	
	if (stbuf[2].itop.ptrw2 == cmpptr)	/* cmp -> cmpm ?? */
	{
		if (numreg[0] && numreg[1] && indir[0] && indir[1])
		{
			stbuf[2].itop.ptrw2 = opcpt = cmpmptr;
		}
	}
	
	if (aesflag && (stbuf[2].itop.ptrw2 == jsrptr || stbuf[2].itop.ptrw2 == bsrptr) &&
		stbuf[ITOP1].itty == ITSY)
	{
		register unsigned short opcode;
		register struct symtab *extsym = stbuf[ITOP1].itop.ptrw2;
		
		if ((opcode = isaes(extsym->name)) != 0)
		{
			if (itwc < (ITOP1 + 1))
			{
				rpterr(_("i.t. overflow"));
				asabort();
			}
			stbuf[2].itop.ptrw2 = opcpt = dcptr;
			stbuf[2].itrl = WORDSIZ;
			stbuf[ITOP1].itty = ITCN;
			stbuf[ITOP1].itrl = ABS;
			stbuf[ITOP1].itop.l = opcode;
		}
	}
	
	if (lopcomma)
		uerr(30); /* invalid data list */
}


/* change add into addq and sub into subq if possible */
static VOID tryquick(NOTHING)
{
	register struct symtab *p;
	register long l;

	if (fchr != ',' || !immed[0])
		return;
	l = ival.l;
	if (itwc != ITOP1 + 1)
	{
		if (itwc != ITOP1 + 2 || stbuf[ITOP1 + 1].itty != ITSP || stbuf[ITOP1 + 1].itop.oper != '-')
			return;
		l = -l;
	}
	p = stbuf[2].itop.ptrw2;
	if (p == moveptr)
	{
		if (explmode && modelen != LONGSIZ)	/* dont change .w or .b */
			return;
		if (l >= -128 && l <= 127)
		{
			stbuf[2].itop.ptrw2 = moveqptr;
			opcpt = moveqptr;
			chmvq++;
		}
		return;
	}
	if (l <= 0 || l > 8)
	{
		return;
	}
	if (p == addptr || p == addiptr)
	{
		stbuf[2].itop.ptrw2 = opcpt = addqptr;
	} else if (p == subptr || p == subiptr)
	{
		stbuf[2].itop.ptrw2 = opcpt = subqptr;
	}
}


/* output it for one operand */
VOID opitoo(NOTHING)
{
	register struct symtab *sp;
	register int16_t h;
	
	symcon = 0;
	if (itype == ITSP)
	{									/* special symbol */
		if (ival.oper == ',' && !plevel)
		{								/* another operand */
			numops++;
			if (!opdix)
				opdix++;
		}
		if (ival.oper == ')')
			symcon = 1;					/* star is multiply */
		if (ival.oper == ' ')
		{								/* end of operands */
			while (fchr != EOLC)		/* ignore rest of statement */
				fchr = gchr();
			return;
		}
		if (ival.oper == EOLC)
			return;
	} else								/* symbol or constant */
	{
		symcon = 1;
	}
	
	if (itwc >= STMAX)
	{									/* it overflow */
		rpterr(_("i.t. overflow"));
		asabort();
	}
	pitw->itty = itype;					/* type of it entry */

	/* put symbol in it buffer */
	if (itype == ITSY)
	{
		sp = lemt(FALSE, sirt);			/* look up it main table */
		if (sp == lmte)					/* first occurrance */
		{
			mmte();
		}
		pitw->itop.ptrw2 = sp;			/* ptr to symbol entry */
		if (!(sp->flags & SYER))		/* is it a register? */
		{
			numsym[opdix]++;
		} else							/* yes, a register */
		{
			if (numreg[opdix] == 0)
				numreg[opdix] = sp->vl1;
			if (itwc > ITOP1)
			{
				if (pitw[-1].itty == ITCN || pitw[-1].itty == ITCW)
				{
					if (sp->vl1 == WORD_ID)
					{
						h = (int16_t)(pitw[-1].itop.l >> 16);
						if (h != 0 && h != -1)
						{
							uerr(44); /* warning: constant out of range */
							nerror--;
						}
						pitw[-1].itty = ITCW;
						numcon[opdix] = 1;
					} else if (sp->vl1 == LONG_ID)
					{
						pitw[-1].itty = ITCN;
						numcon[opdix] = 2;
					}
				}	
			}
		}
		itwc++;							/* count entries in it buffer */
		pitw++;
		return;
	} else if (itype == ITCN)
	{
		h = (int16_t)(ival.l >> 16);
		if (!shortadr || (h != 0 && h != -1))
		{
			numcon[opdix] = 2;
		} else if (!numcon[opdix])
		{
			numcon[opdix] = 1;
			itype = pitw->itty = ITCW;
		}
		if (numops == 1)
			tryquick();
		reloc = ABS;
	}

	/* special characters and constants */
	pitw->itop.p = ival.p;
	pitw->itrl = reloc;
	itwc++;
	pitw++;
}


static struct {
	char name[4];
	short val;
} const regnames[] = {
	{ "R0", 0 },
	{ "R1", 1 },
	{ "R2", 2 },
	{ "R3", 3 },
	{ "R4", 4 },
	{ "R5", 5 },
	{ "R6", 6 },
	{ "R7", 7 },
	{ "R8", 8 },
	{ "R9", 9 },
	{ "R10", 10 },
	{ "R11", 11 },
	{ "R12", 12 },
	{ "R13", 13 },
	{ "R14", 14 },
	{ "R15", 15 },
	{ "D0", 0 },
	{ "D1", 1 },
	{ "D2", 2 },
	{ "D3", 3 },
	{ "D4", 4 },
	{ "D5", 5 },
	{ "D6", 6 },
	{ "D7", 7 },
	{ "A0", 8 },
	{ "A1", 9 },
	{ "A2", 10 },
	{ "A3", 11 },
	{ "A4", 12 },
	{ "A5", 13 },
	{ "A6", 14 },
	{ "A7", 15 },
	{ "SP", 15 },
	{ "CCR", CCR },
	{ "SR", SR },
	{ "USP", USP },
	{ ".B", BYTE_ID },
	{ ".W", WORD_ID },
	{ ".L", LONG_ID },
	{ "PC", PC },
	{ "SFC", SFC },
	{ "DFC", DFC },
	{ "VSR", VBR },
	{ "VBR", VBR },

	{ "r0", 0 },
	{ "r1", 1 },
	{ "r2", 2 },
	{ "r3", 3 },
	{ "r4", 4 },
	{ "r5", 5 },
	{ "r6", 6 },
	{ "r7", 7 },
	{ "r8", 8 },
	{ "r9", 9 },
	{ "r10", 10 },
	{ "r11", 11 },
	{ "r12", 12 },
	{ "r13", 13 },
	{ "r14", 14 },
	{ "r15", 15 },
	{ "d0", 0 },
	{ "d1", 1 },
	{ "d2", 2 },
	{ "d3", 3 },
	{ "d4", 4 },
	{ "d5", 5 },
	{ "d6", 6 },
	{ "d7", 7 },
	{ "a0", 8 },
	{ "a1", 9 },
	{ "a2", 10 },
	{ "a3", 11 },
	{ "a4", 12 },
	{ "a5", 13 },
	{ "a6", 14 },
	{ "a7", 15 },
	{ "sp", 15 },
	{ "ccr", CCR },
	{ "sr", SR },
	{ "usp", USP },
	{ ".b", BYTE_ID },
	{ ".w", WORD_ID },
	{ ".l", LONG_ID },
	{ "pc", PC },
	{ "sfc", SFC },
	{ "dfc", DFC },
	{ "vsr", VBR },
	{ "vbr", VBR },
};


static VOID equreg(P(const char *) name, P(int) val)
PP(const char *name;)
PP(int val;)
{
	register struct symtab *ptr;
	
	pack(name, lmte);
	ptr = lemt(FALSE, sirt);
	if (ptr != lmte)
	{
		uerr(1, name);	/* label redefined */
		asabort();
	}
	mmte();
	ptr->flags = SYDF | SYEQ | SYER | SYIN;
	ptr->vl1 = val;
}


static struct symtab *mkopd(P(const char *) name, P(int) formt, P(unsigned short) val)
PP(const char *name;)
PP(int formt;)
PP(unsigned short val;)
{
	register struct symtab *ptr;
	
	pack(name, lmte);
	ptr = lemt(FALSE, oirt);
	if (ptr != lmte)
	{
		uerr(5, name); /* opcode redefined */
		asabort();
	}
	mmte();
	ptr->flags = formt | SYIN;
	ptr->vl1 = val;
	return ptr;
}


static VOID usage(NOTHING)
{
	rpterr(_("Usage: as68 [-p] [-u] [-l] [-n] [-t] [-f d:] sourcefile"));
	rpterr(_("Options:"));
	rpterr(_("    -p   produce a listing"));
	rpterr(_("    -u   make undefined symbols external"));
	rpterr(_("    -n   no branch optimization"));
	rpterr(_("    -l   use long addresses (default)"));
	rpterr(_("    -a   use short addresses"));
	rpterr(_("    -t   generating code suitable for the 68010"));
	rpterr(_("    -f   redirect temp files to directory"));
	exit(EXIT_FAILURE);
}


#include "../common/linux/libcmain.h"

#ifdef __GNUC__
/* gcc will optimize the reference in the macro away */
extern struct symtab *opformat_out_of_range(void);
extern struct symtab *directive_out_of_range(void);
#define MKOPD(name, formt, val) ((formt) > OPFF ? opformat_out_of_range() : mkopd(name, formt, val))
#define MDEMT(name, dir) ((dir) >= DIRECT ? directive_out_of_range() : mdemt(name, dir))
#else
#define MKOPD(name, formt, val) mkopd(name, formt, val)
#define MDEMT(name, dir) mdemt(name, dir)
#endif


int main(P(int) argc, P(char **) argv)
PP(int argc;)
PP(char **argv;)
{
	register short i;
	char *ofilename;
	char *arg;

#ifdef __ALCYON__
	/* symbols etoa and ftoa are unresolved */
	asm("xdef _etoa");
	asm("_etoa equ 0");
	asm("xdef _ftoa");
	asm("_ftoa equ 0");
#endif

	nerror = 0;

	signal(SIGINT, rubout);
	signal(SIGQUIT, rubout);
	signal(SIGHUP, rubout);
	signal(SIGTERM, rubout);

	pitix = 0;

	if (argc <= 1)
		usage();

	shortadr = 0;						/* long addresses... */
	aesflag = 0;
	ofilename = NULL;
	
	i = 1;
	while (argv[i][0] == '-')
	{									/* may be print or initialize */
		arg = argv[i++];
		
		switch (arg[1])
		{
		case 'a':						/* short addresses only */
			shortadr = -1;
			break;

		case 'i':						/* initialize the assembler */
			/* ignored; no longer needed */
			break;

		case 'p':						/* produce a listing */
			prtflg++;
			break;

		case 'u':						/* make undefined symbols external */
			undflg++;
			break;

		case 'N':						/* no branch optimization */
		case 'n':
			didorg++;
			break;

		case 'L':						/* OBSOLETE, long addresses only */
		case 'l':
			shortadr = 0;
			break;

		case 'T':						/* generating code suitable for the 68010 */
		case 't':
			m68010++;
			break;

		case 'f':						/* Redirect temp files */
			tdname = argv[i++];
			break;

		case 's':						/* Change symbol table prefix */
			i++;
			/* ignored; no longer needed */
			break;

		case 'A':
			aesflag = 1;
			lineftbl = &arg[2];
			break;

		case 'o':
			ofilename = argv[i++];
			break;

		default:
			usage();
		}
	}
	if (i >= argc)
		usage();
		
	if (aesflag)
		readlineftab();

	/* Remember source filename */
	sfname = argv[i];
	/* open source file */
	ifn = openfi(argv[i], "r");
	/* create relocatable object file name */
	if (ofilename)
		strcpy(ldfn, ofilename);
	else
		setldfn(argv[i]);
	/* open loader file */
	lfil = openfi(ldfn, "wb");
	
	/* get a temp file for it */
	itfn = gettempf(itfilnam);
	/* temp for text relocation bits */
	trfil = gettempf(trfilnam);
	/* temp for data binary */
	dafil = gettempf(dafilnam);
	/* temp for data relocation bits */
	drfil = gettempf(drfilnam);
	
	initsy();
	
	/* make entries in main table for directives */
	MDEMT("opd", DIR_OPD);			/* opcode definition */
	endptr = MDEMT("end", DIR_END);	/* end statement */
	MDEMT("data", DIR_DSECT);		/* dsect directive(code DATA based) */
	MDEMT("text", DIR_PSECT);		/* psect directive(code TEXT based) */
	MDEMT("equ", DIR_EQU);			/* equate */
	MDEMT("set", DIR_SET);			/* .set - same as .equ */
	MDEMT("ascii", DIR_ASCII);		/* define ascii string */
	dcptr = MDEMT("dc", DIR_DC);	/* define constant */
	MDEMT("globl", DIR_GLOBL);		/* define global (public) symbols */
	MDEMT("xdef", DIR_GLOBL);		/* define global (public) symbols */
	MDEMT("xref", DIR_GLOBL);		/* define global (public) symbols */
	MDEMT("comm", DIR_COMM);		/* define external symbols */
	MDEMT("bss", DIR_BSS);			/* block storage based */
	MDEMT("ds", DIR_DS);			/* block storage based */
	evenptr = MDEMT("even", DIR_EVEN);	/* round pc */
	MDEMT("~.yxzorg", DIR_ORG);		/* internal, *= */
	orgptr = MDEMT("org", DIR_ORG);	/* org location, also *= */
	MDEMT("mask2", DIR_MASK2);		/* assemble for mask2, ignore */
	MDEMT("reg", DIR_REGEQU);		/* register equate */
	MDEMT("dcb", DIR_DCB);				/* define block */
	MDEMT("comline", DIR_COMLINE);	/* command line */
	MDEMT("idnt", DIR_IDNT);		/* relocateable id record, ignore */
	MDEMT("offset", DIR_OFFSET);	/* define offsets */
	MDEMT("section", DIR_SECTION);	/* define sections */
	MDEMT("ifeq", DIR_IFEQ);		/* ca if expr = 0 */
	MDEMT("ifne", DIR_IFNE);		/* ca if expr != 0 */
	MDEMT("iflt", DIR_IFLT);		/* ca if expr < 0 */
	MDEMT("ifle", DIR_IFLE);		/* ca if expr <= 0 */
	MDEMT("ifgt", DIR_IFGT);		/* ca if expr > 0 */
	MDEMT("ifge", DIR_IFGE);		/* ca if expr >= 0 */
	MDEMT("endc", DIR_ENDC);		/* end ca */
	MDEMT("ifc", DIR_IFC);			/* ca if string compare */
	MDEMT("ifnc", DIR_IFNC);		/* ca if not string compare */
	MDEMT("opt", DIR_OPT);			/* ignored, assemb options */
	MDEMT("ttl", DIR_TTL);			/* ttl define, ignore */
	MDEMT("page", DIR_PAGE);		/* page define, ignore */
	
	/* make entries in main table for registers */
	for (i = 0; i < (short)(sizeof(regnames) / sizeof(regnames[0])); i++)
		equreg(regnames[i].name, regnames[i].val);
	
	/* make entries in opcode table */

	MKOPD("abcd",    FORMAT_ABCD, 0140400);
	addptr = MKOPD("add",    FORMAT_2EA, 0150000);
	addaptr = MKOPD("adda",    FORMAT_ADDA, 0150000);
	addiptr = MKOPD("addi",    FORMAT_ADDI, 0003000);
	addqptr = MKOPD("addq",    FORMAT_ADDQ, 0050000);
	MKOPD("inc",    FORMAT_INC, 0050000);
	MKOPD("addx",   FORMAT_ADDX, 0150400);
	andptr = MKOPD("and",    FORMAT_2EA, 0140000);
	andiptr = MKOPD("andi",    FORMAT_ADDI, 0001000);
	MKOPD("asl",     FORMAT_ASL, 0160400);
	MKOPD("asr",     FORMAT_ASL, 0160000);
	MKOPD("bcc",     FORMAT_RELBR, 0062000);
	MKOPD("bcs",     FORMAT_RELBR, 0062400);
	MKOPD("beq",     FORMAT_RELBR, 0063400);
	MKOPD("bze",     FORMAT_RELBR, 0063400);
	MKOPD("bge",     FORMAT_RELBR, 0066000);
	MKOPD("bgt",     FORMAT_RELBR, 0067000);
	MKOPD("bhi",     FORMAT_RELBR, 0061000);
	MKOPD("bhis",    FORMAT_RELBR, 0062000);
	MKOPD("bhs",     FORMAT_RELBR, 0062000);
	MKOPD("ble",     FORMAT_RELBR, 0067400);
	MKOPD("blo",     FORMAT_RELBR, 0062400);
	MKOPD("bls",     FORMAT_RELBR, 0061400);
	MKOPD("blos",    FORMAT_RELBR, 0061400);
	MKOPD("blt",     FORMAT_RELBR, 0066400);
	MKOPD("bmi",     FORMAT_RELBR, 0065400);
	MKOPD("bne",     FORMAT_RELBR, 0063000);
	MKOPD("bnz",     FORMAT_RELBR, 0063000);
	MKOPD("bpl",     FORMAT_RELBR, 0065000);
	MKOPD("bvc",     FORMAT_RELBR, 0064000);
	MKOPD("bvs",     FORMAT_RELBR, 0064400);
	MKOPD("bchg",    FORMAT_BTST, 0000100);
	MKOPD("bclr",    FORMAT_BTST, 0000200);
	MKOPD("bra",     FORMAT_RELBR, 0060000);
	MKOPD("bt",      FORMAT_RELBR, 0060000);
	MKOPD("bset",    FORMAT_BTST, 0000300);
	bsrptr = MKOPD("bsr",    FORMAT_RELBR, 0060400);
	MKOPD("btst",    FORMAT_BTST, 0000000);
	MKOPD("chk",    FORMAT_CMP, 0040600);
	MKOPD("clr",    FORMAT_CLR, CLRVAL);
	cmpptr = MKOPD("cmp",    FORMAT_CMP, 0130000);
	cmpaptr = MKOPD("cmpa",    FORMAT_ADDA, 0130000);
	cmpiptr = MKOPD("cmpi",    FORMAT_ADDI, 0006000);
	cmpmptr = MKOPD("cmpm",    FORMAT_CMPM, 0130410);
	MKOPD("dbcc",   FORMAT_DBCC, 0052310);
	MKOPD("dbcs",   FORMAT_DBCC, 0052710);
	MKOPD("dblo",   FORMAT_DBCC, 0052710);
	MKOPD("dbeq",   FORMAT_DBCC, 0053710);
	MKOPD("dbze",   FORMAT_DBCC, 0053710);
	MKOPD("dbra",   FORMAT_DBCC, 0050710);
	MKOPD("dbf",    FORMAT_DBCC, 0050710);
	MKOPD("dbge",   FORMAT_DBCC, 0056310);
	MKOPD("dbgt",   FORMAT_DBCC, 0057310);
	MKOPD("dbhi",   FORMAT_DBCC, 0051310);
	MKOPD("dbhs",   FORMAT_DBCC, 0051310);
	MKOPD("dble",   FORMAT_DBCC, 0057710);
	MKOPD("dbls",   FORMAT_DBCC, 0051710);
	MKOPD("dblt",   FORMAT_DBCC, 0056710);
	MKOPD("dbmi",   FORMAT_DBCC, 0055710);
	MKOPD("dbne",   FORMAT_DBCC, 0053310);
	MKOPD("dbnz",   FORMAT_DBCC, 0053310);
	MKOPD("dbpl",   FORMAT_DBCC, 0055310);
	MKOPD("dbt",    FORMAT_DBCC, 0050310);
	MKOPD("dbvc",   FORMAT_DBCC, 0054310);
	MKOPD("dbvs",   FORMAT_DBCC, 0054710);
	MKOPD("divs",    FORMAT_DIV, 0100700);
	MKOPD("divu",    FORMAT_DIV, 0100300);
	eorptr = MKOPD("eor",    FORMAT_EOR, 0130000);
	eoriptr = MKOPD("eori",    FORMAT_ADDI, 0005000);
	exgptr = MKOPD("exg",    FORMAT_EXG, 0140400);
	MKOPD("ext",    FORMAT_EXT, 0044000);
	MKOPD("jmp",     FORMAT_JMP, 0047300);
	jsrptr = MKOPD("jsr",    FORMAT_JMP, 0047200);
	MKOPD("illegal",   FORMAT_NONE, 0045374);
	MKOPD("lea",    FORMAT_LEA, 0040700);
	MKOPD("link",   FORMAT_LINK, 0047120);
	MKOPD("lsr",     FORMAT_ASL, 0160010);
	MKOPD("lsl",     FORMAT_ASL, 0160410);
	moveptr = MKOPD("move",    3, 0000000);
	MKOPD("movea",   FORMAT_MOVE, 0000100);
	MKOPD("movec",  FORMAT_MOVEC, 0047172);
	MKOPD("movem",  FORMAT_MOVEM, 0044200);
	MKOPD("movep",  FORMAT_MOVEP, 0000010);
	moveqptr = MKOPD("moveq",    FORMAT_MOVEQ, 0070000);
	MKOPD("moves",  FORMAT_MOVEC, 0007000);
	MKOPD("muls",    FORMAT_DIV, 0140700);
	MKOPD("mulu",    FORMAT_DIV, 0140300);
	MKOPD("nbcd",   FORMAT_SCC, 0044000);
	MKOPD("neg",    FORMAT_CLR, 0042000);
	MKOPD("negx",   FORMAT_CLR, 0040000);
	nopptr = MKOPD("nop",    FORMAT_NONE, 0047161);
	MKOPD("not",    FORMAT_CLR, 0043000);
	orptr = MKOPD("or",    FORMAT_2EA, 0100000);
	oriptr = MKOPD("ori",    FORMAT_ADDI, 0000000);
	MKOPD("pea",    FORMAT_PEA, 0044100);
	MKOPD("reset",   FORMAT_NONE, 0047160);
	MKOPD("rol",     FORMAT_ASL, 0160430);
	MKOPD("ror",     FORMAT_ASL, 0160030);
	MKOPD("roxl",    FORMAT_ASL, 0160420);
	MKOPD("roxr",    FORMAT_ASL, 0160020);
	MKOPD("rtd",    FORMAT_STOP, 0047164);
	MKOPD("rte",     FORMAT_NONE, 0047163);
	MKOPD("rtr",     FORMAT_NONE, 0047167);
	MKOPD("rts",     FORMAT_NONE, 0047165);
	MKOPD("sbcd",    FORMAT_ABCD, 0100400);
	MKOPD("scc",    FORMAT_SCC, 0052300);
	MKOPD("shs",    FORMAT_SCC, 0052300);
	MKOPD("scs",    FORMAT_SCC, 0052700);
	MKOPD("slo",    FORMAT_SCC, 0052700);
	MKOPD("seq",    FORMAT_SCC, 0053700);
	MKOPD("sze",    FORMAT_SCC, 0053700);
	MKOPD("sf",     FORMAT_SCC, 0050700);
	MKOPD("sge",    FORMAT_SCC, 0056300);
	MKOPD("sgt",    FORMAT_SCC, 0057300);
	MKOPD("shi",    FORMAT_SCC, 0051300);
	MKOPD("sle",    FORMAT_SCC, 0057700);
	MKOPD("sls",    FORMAT_SCC, 0051700);
	MKOPD("slt",    FORMAT_SCC, 0056700);
	MKOPD("smi",    FORMAT_SCC, 0055700);
	MKOPD("sne",    FORMAT_SCC, 0053300);
	MKOPD("snz",    FORMAT_SCC, 0053300);
	MKOPD("spl",    FORMAT_SCC, 0055300);
	MKOPD("st",     FORMAT_SCC, 0050300);
	MKOPD("svc",    FORMAT_SCC, 0054300);
	MKOPD("svs",    FORMAT_SCC, 0054700);
	MKOPD("stop",   FORMAT_STOP, 0047162);
	subptr = MKOPD("sub",    FORMAT_2EA, 0110000);
	subaptr = MKOPD("suba",    FORMAT_ADDA, 0110000);
	subiptr = MKOPD("subi",    FORMAT_ADDI, 0002000);
	subqptr = MKOPD("subq",    FORMAT_ADDQ, 0050400);
	MKOPD("dec",    FORMAT_INC, 0050400);
	MKOPD("subx",   FORMAT_ADDX, 0110400);
	MKOPD("swap",   FORMAT_SWAP, 0044100);
	MKOPD("tas",    FORMAT_SCC, 0045300);
	MKOPD("trap",   FORMAT_TRAP, 0047100);
	MKOPD("trapv",   FORMAT_NONE, 0047166);
	MKOPD("tst",    FORMAT_CLR, 0045000);
	MKOPD("unlk",   FORMAT_EXT, 0047130);
	
	rlflg = TEXT;						/* code initially TEXT based */
	inoffset = 0;						/* not in offset mode */
	loctr = 0;							/* no generated code */
	ca = 0;								/* depth of conditional assembly */
	extindx = 0;						/* no external symbols yet */
	vextno = 0;
	p2flg = 0;							/* pass 1 */
	ca_true = 1;						/* true unless in side false case */
	absln = 1;
	fchr = gchr();						/* get first char */

	mloop();
	return EXIT_SUCCESS;
}
