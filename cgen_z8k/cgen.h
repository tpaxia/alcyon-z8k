/*
	Copyright 1983
	Alcyon Corporation
	8716 Production Ave.
	San Diego, Ca.  92121

	Retargeted for Zilog Z8002 (non-segmented)
*/

#include "../common/linux/libcwrap.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../util/util.h"

#define BINEXACT 1

#include "icode.h"
#include "cskel.h"

#define _(x) x

/*
 * Z8002: INC/DEC immediate range is 1-16
 * (QUICKVAL is used by ADD→SUB optimization in ucodegen for negating small constants)
 */
#define	QUICKVAL		16
/*
 * Z8002: Frame pointer is R14 by convention
 */
#define	LEP				14
#define	FORCC			1
#define	FOREFF			2
#define	FORSTACK		3
#define	FORCREG			4
#define	FORSP			5
#define	FORREG			4
/*
 * Z8002: Compiler temporaries R0-R2
 */
#define	HICREG			2
/*
 * Z8002: Uniform register file — no data/address register split.
 * Set AREGLO to 16 (beyond R15) so ISAREG() is always false.
 */
#define	AREGLO			16

/* tcopy + outaexpr flags - generate prefix operators, postfix operators */
#define	A_DOPRE			1
#define	A_DOPOST		2
#define	A_DOIMMED		4
#define	A_NOIMMED		0

#define PATHSIZE		128

extern char const brtab[][2];
extern short const invrel[];
extern short const swaprel[];
extern const char *const strtab[];
extern const char *const opname[];


#ifdef VAX11
#define LW_LITTLE 1
#endif
#ifdef __i386__
#define LW_LITTLE 1
#endif
#ifdef __x86_64__
#define LW_LITTLE 1
#endif
#ifdef __arm__
#define LW_LITTLE 1
#endif
#ifdef __aarch64__
#define LW_LITTLE 1
#endif
#ifdef LW_LITTLE
struct words { short loword; short hiword; };
#else
struct words { short hiword; short loword; };
#endif

/* local symbol terminal node */
struct xlsym {
	short sc;						/* storage class */
	int32_t offset;					/* register offset */
	short reg;						/* register number */
	short label;					/* label number if static */
};

/* external symbol reference node */
struct xesym {
	short sc;						/* storage class */
	int32_t offset;					/* register offset */
	short reg;						/* register number */
	char symbol[SSIZE];				/* symbol name if external */
};

/* indexed symbol node - Z8002 uses base+index addressing */
struct xxsym {
	short sc;						/* storage class */
	int32_t offset;					/* register offset */
	short reg;						/* register number */
	short xreg;
	short xtype;
};

union tval {
	struct {
		struct tnode *left;			/* left sub-tree */
		struct tnode *right;		/* right sub-tree (undefined if unary) */
	} t;
	struct words w;
	short s;						/* value or label number */
	int32_t l;						/* value or label number */
	struct xlsym lsym;
	struct xesym esym;
	struct xxsym xsym;
};


/* operator tree node for unary and binary operators */
struct tnode {
	short t_op;						/* operator */
	short t_type;					/* data type of result */
	short t_su;						/* Sethy-Ullman number */
	short t_ssp;
	union tval v;
};

#define t_value v.s
#define t_lvalue v.l
#define t_left v.t.left
#define t_right v.t.right
#define t_sc v.lsym.sc
#define t_offset v.lsym.offset
#define t_reg v.lsym.reg
#define t_label v.lsym.label
#define t_symbol v.esym.symbol
#define t_xreg v.xsym.xreg
#define t_xtype v.xsym.xtype

extern struct tnode null;

/* Code generation argument flags */
extern short m68010; /* bool: unused on Z8002 but kept for interface compat */
extern short dflag; /* bool: include line numbers in assembly output */
extern short cflag; /* bool: debug code reader */
extern short eflag; /* bool: debug skeleton expansion */
extern short mflag; /* bool: debug skeleton match */
extern short oflag; /* bool: debug operators */
extern short gflag; /* bool: generate line labels for cdb */
extern short lflag; /* bool: assume long address variables */
extern short aesflag; /* bool: unused on Z8002 */

/* expression tree storage */
#define EXPSIZE     4096
extern char exprarea[EXPSIZE];
extern char *opap;

/* Miscellaneous variables */
extern short lineno;
extern short errcnt;
extern short onepass;
extern short bol;
extern short const opinfo[];
extern short nextlabel;
extern char const optab[][6];
extern const char *const mnemonics[];
extern const struct skeleton *const codeskels[];
extern short stacksize;

/* general define macros */
#define ISTYPEDEF(sp)		(sp->s_attrib & STYPEDEF)
#define WALIGN(add)			((add + 1) & (~1))
#define ISARRAY(type)		(((type) & SUPTYP) == ARRAY)
#define ISFUNCTION(type)	(((type) & SUPTYP) == FUNCTION)
#define ISPOINTER(type)		(((type) & SUPTYP) == POINTER)
#define ISFLOAT(type)		((type) == FLOAT)
#define BTYPE(type)			((type) & TYPE)
#define SUPTYPE(type)		((type) & SUPTYP)
#define ISALLTYPE(type)		((type) & (SUPTYP|TYPE))
#define OPPRIORITY(op)		(opinfo[op] & OPPRI)
#define ISASGOP(op)			((opinfo[op] & OPASSIGN) != 0)
#define RELOP(op)			((opinfo[op] & OPREL) != 0)
#define COMOP(op)			((opinfo[op] & OPCOM) != 0)
#define LINTEGRAL(op)		((opinfo[op] & OPLWORD) != 0)
#define RINTEGRAL(op)		((opinfo[op] & OPRWORD) != 0)
#define RASOP(op)			((opinfo[op] & OPRAS) != 0)
#define BINOP(op)			((opinfo[op] & OPBIN) != 0)
#define UNARYOP(op)			((opinfo[op] & (OPBIN|OPTERM)) == 0)
#define LEAFOP(op)			((opinfo[op] & OPTERM) != 0)
#define LVALOP(op)			((opinfo[op] & OPLVAL) != 0)
#define COMMOP(op)			((opinfo[op] & OPCOM) != 0)
#define CONVOP(op)			((opinfo[op] & OPCONVS) != 0)
#define SIMPLE_TYP(typ)		((typ >= CHAR) && (typ <= DOUBLE))

#undef MAX
#undef MIN
#define	MAX(a,b)		((a) > (b) ? (a) : (b))
#define MIN(a,b)		((a) < (b) ? (a) : (b))

/*
 * Z8002-specific output macros.
 *
 * LONGTYPE: true for LONG/ULONG/FLOAT but NOT pointers (which are 16-bit on Z8002).
 * This is the key distinction from LONGORPTR on the 68000.
 */
#define LONGTYPE(type)	((type) == LONG || (type) == ULONG || ISFLOAT(type))

/* outgoto - output "jp t,L[labno]" (Z8002 unconditional jump) */
#if BINEXACT
#define OUTGOTO(lab)	if (lab > 0) oprintf("\tjp L%d\n",lab)
/* outlab - output "L[labno]:" */
#define OUTLAB(lab)		if (lab > 0) oprintf("L%d:\n",lab)
#else
#define OUTGOTO(lab)	if ((lab) > 0) oprintf("\tjp L%d\n",lab)
#define OUTLAB(lab)		if ((lab) > 0) oprintf("L%d:\n",lab)
#endif

/*
 * outext - signed word→long extension
 * Z8002: Value is in R(reg). Need sign-extended 32-bit in RR(reg&~1).
 * Copy to odd register of pair, then sign-extend.
 * exts RRn sign-extends R(n+1) into R(n).
 */
#define OUTEXT(reg)		oprintf("\tld R%d,R%d\n\texts RR%d\n",(reg)|1,(reg),(reg)&~1)
/*
 * outuext - unsigned word→long extension
 * Z8002: Value is in R(reg). Need zero-extended 32-bit in RR(reg&~1).
 * Copy to odd register of pair, then clear even register.
 */
#define OUTUEXT(reg)	oprintf("\tld R%d,R%d\n\tclr R%d\n",(reg)|1,(reg),(reg)&~1)
/* outswap - Z8002 has no swap; we don't need it (MOD handled differently) */
#define OUTSWAP(reg)	/* no-op on Z8002 */
/* outaddr - output "add [type] R1 R2" instruction */
#define OUTADDR(r1,r2,tp)	outrr("add",r1,r2,(tp))
/*
 * Z8002: No direct cc save/restore instructions.
 * The scodegen post-increment CC handling path will fall through to outcmp0 instead.
 */
#define OUTSRSAVE(reg)	/* no-op on Z8002 */
#define OUTCCSAVE(reg)	/* no-op on Z8002 */
#define OUTCCRESTORE(reg)	/* no-op on Z8002 */

/* basetype - get the btype info sans unsigned */
#define BASETYPE(type)	((type == UNSIGNED) ? INT : type)
#define UNSIGN(type)	((type) == UNSIGNED)
/*
 * Z8002: pointers are 16-bit (same as int), so LONGORPTR only matches LONG types.
 * For skeleton matching, we still need LONGORPTR for the original code paths.
 */
#define LONGORPTR(type)	(LONGTYPE(type) || ((type) & SUPTYP))
#define UNORPTR(type)	(type == UNSIGNED || ((type) & SUPTYP))

/*
 * Z8002: Uniform register file — all registers are general-purpose.
 * DREG/AREG are identity operations. ISAREG is always false.
 */
#define DREG(reg)		(reg)
#define AREG(reg)		(reg)
#define ISAREG(reg)		(0)
#define ISDREG(reg)		(1)
#define ISREG(tp)		((tp)->t_op == SYMBOL && (tp)->t_sc == REGISTER)

#define CONSTZERO(ltyp,p) ((ltyp && !p->t_lvalue) || (!ltyp && !p->t_value))
#define SETVAL(ltyp,p,val) if (ltyp) p->t_lvalue = val; else p->t_value = val

#define BFOFFS(su) (((su) >> 8) & 0xff)
#define BFLEN(su)  (((su)     ) & 0xff)

#ifdef DEBUG
#	define PUTEXPR(cond,id_str,node_ptr)	if (cond) putexpr(id_str,node_ptr)
#else
#	define PUTEXPR(cond,id_str,node_ptr)
#endif

/* Functions pre-declared */

/*
 * canon.c
 */
struct tnode *canon PROTO((struct tnode *tp));
struct tnode *constant PROTO((struct tnode *tp, short *lconst));
int indexreg PROTO((struct tnode *tp));
int onebit PROTO((int32_t val));

/*
 * codegen.c
 */
int scodegen PROTO((struct tnode *tp, int cookie, int reg));
short codegen PROTO((struct tnode *tp, int cookie, int reg));
struct tnode *coffset PROTO((struct tnode *tp));
VOID condbr PROTO((struct tnode *tp, int dir, int lab, int reg));

/*
 * interf.c
 */
VOID outexpr PROTO((struct tnode *tp));
VOID outifgoto PROTO((struct tnode *tp, int dir, int lab));
VOID outcforreg PROTO((struct tnode *tp));
VOID outinit PROTO((struct tnode *tp));
struct tnode *snalloc PROTO((int type, int sc, int32_t offset, int dp, int ssp));
VOID outline PROTO((NOTHING));


/*
 * main.c
 */
VOID error PROTO((const char *s, ...)) __attribute__((format(__printf__, 1, 2)));
VOID warning PROTO((const char *s, ...)) __attribute__((format(__printf__, 1, 2)));
VOID fatal PROTO((const char *s, ...)) __attribute__((format(__printf__, 1, 2)));
struct tnode *tnalloc PROTO((int op, int type, int info, int dummy, struct tnode *left, struct tnode *right));
struct tnode *cnalloc PROTO((int type, int value));
struct tnode *lcnalloc PROTO((int type, int32_t value));
struct tnode *fpcnalloc PROTO((int type, int32_t value));
struct tnode *talloc PROTO((int size));

VOID oputchar PROTO((char c));
VOID oprintf PROTO((const char *s, ...)) __attribute__((format(__printf__, 1, 2)));

/*
 * putexpr.c
 */
VOID putexpr PROTO((const char *name, struct tnode *tp));
VOID puttsu PROTO((struct tnode *tp));


/*
 * smatch.c
 */
int expand PROTO((struct tnode *tp, int cookie, int freg, const struct skeleton *skp));
const struct skeleton *match PROTO((struct tnode *tp, int cookie, int reg));


/*
 * sucomp.c
 */
short sucomp PROTO((struct tnode *tp, int nregs, int flag));


/*
 * util.c
 */
struct tnode *xnalloc PROTO((int type, int ar, int32_t off, int xr, int xt));
struct tnode *tcopy PROTO((struct tnode *tp, int autof));
VOID outaexpr PROTO((struct tnode *tp, int flags));
VOID outtype PROTO((int type));
VOID outatype PROTO((int type));
VOID outextend PROTO((struct tnode *tp, int type, int reg));
VOID outrr PROTO((const char *ins, int r1, int r2, struct tnode *tp));
VOID outmovr PROTO((int r1, int r2, struct tnode *tp));
VOID outcreg PROTO((int reg, int type));
VOID outcmp0 PROTO((int reg, struct tnode *tp));
VOID outrpush PROTO((int reg, struct tnode *tp, int pflag));
int outdbra PROTO((int dir, int op, struct tnode *ltp, struct tnode *rtp, int lab));
struct tnode *cenalloc PROTO((int type, int sc, const char *sym));
VOID popstack PROTO((int nb));
VOID outcmpm PROTO((struct tnode *tp));
