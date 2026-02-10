/*
	Copyright 1983
	Alcyon Corporation
	8716 Production Ave.
	San Diego, Ca.  92121

	Retargeted for Zilog Z8002 (non-segmented)
*/

#include "cgen.h"




/* talloc - allocate expression tree area */
struct tnode *talloc(P(int) size)
PP(int size;)
{
	register char *p;

	p = opap;
	if (p + size >= &exprarea[EXPSIZE])
		fatal(_("expression too complex"));
	opap = p + size;
	return (struct tnode *)p;
}


/*
 * snalloc - code generator symbol node allocation
 * This might be coalesced into parser snalloc.
 */
struct tnode *snalloc(P(int) type, P(int) sc, P(int32_t) offset, P(int) dp, P(int) ssp)
PP(int type;)								/* type of symbol */
PP(int sc;)									/* storage class */
PP(int32_t offset;)							/* offset from Local Environment Ptr */
PP(int dp;)									/* for compatibility with parser */
PP(int ssp;)								/* for compatibility with parser */
{
	register struct tnode *sp;

	sp = talloc(sizeof(struct tnode) - sizeof(union tval) + sizeof(struct xlsym));
	sp->t_op = SYMBOL;
	sp->t_type = type;
	sp->t_su = dp;
	sp->t_ssp = ssp;
	sp->t_sc = sc;
	switch (sc)
	{
	case STATIC:
		sp->t_offset = 0;
		sp->t_reg = 0;
		sp->t_label = offset;
		break;

	case REGISTER:
		sp->t_offset = 0;
		sp->t_reg = offset;
		sp->t_label = 0;
		break;

	case AUTO:
		sp->t_sc = REGOFF;
		sp->t_offset = offset;
		sp->t_reg = LEP;
		sp->t_label = 0;
		break;

	default:
		sp->t_offset = offset;
		sp->t_reg = 0;
		sp->t_label = 0;
		break;
	}
	return sp;
}


/*
 * tnalloc - allocate binary expression tree node
 * returns ptr to node made.
 */
struct tnode *tnalloc(P(int) op, P(int) type, P(int) info, P(int) dummy, P(struct tnode *) left, P(struct tnode *) right)
PP(int op;)									/* operator */
PP(int type;)								/* resultant node type */
PP(int info;)								/* info field */
PP(int dummy;)								/* dummy field - used to match pass1 args */
PP(struct tnode *left;)						/* left sub-tree */
PP(struct tnode *right;)					/* righst sub-tree */
{
	register struct tnode *tp;

	UNUSED(dummy);
	tp = talloc(sizeof(struct tnode) - sizeof(union tval) + sizeof(tp->v.t));
	tp->t_op = op;
	tp->t_type = type;
	tp->t_su = info;						/* info for bit-field & condbr's */
	tp->t_ssp = 0;
	tp->t_left = left;
	tp->t_right = right;
	return tp;
}


/* cnalloc - allocate constant expression tree node */
struct tnode *cnalloc(P(int) type, P(int) value)
PP(int type;)								/* type of constant */
PP(int value;)								/* value of constant */
{
	register struct tnode *cp;

	cp = talloc(sizeof(struct tnode) - sizeof(union tval) + sizeof(cp->t_value));
	cp->t_op = CINT;
	cp->t_type = type;
	cp->t_su = 0;
	cp->t_ssp = 0;
	cp->t_value = value;
	return cp;
}


/* lcnalloc - allocate long constant expression tree node */
struct tnode *lcnalloc(P(int) type, P(int32_t) value)
PP(int type;)								/* type of constant */
PP(int32_t value;)							/* value of constant */
{
	register struct tnode *cp;

	cp = talloc(sizeof(struct tnode) - sizeof(union tval) + sizeof(cp->t_lvalue));
	cp->t_op = CLONG;
	cp->t_type = type;
	cp->t_su = 0;
	cp->t_ssp = 0;
	cp->t_lvalue = value;
	return cp;
}


/* fpcnalloc - allocate constant expression tree node */
struct tnode *fpcnalloc(P(int) type, P(int32_t) value)
PP(int type;)								/* type of constant */
PP(int32_t value;)							/* value of constant */
{
	register struct tnode *cp;

	cp = talloc(sizeof(*cp) - sizeof(union tval) + sizeof(cp->t_lvalue));
	cp->t_op = CFLOAT;
	cp->t_type = type;
	cp->t_su = 0;
	cp->t_ssp = 0;
	cp->t_lvalue = value;
	return cp;
}


/* xnalloc - allocate address-indexed node */
struct tnode *xnalloc(P(int) type, P(int) ar, P(int32_t) off, P(int) xr, P(int) xt)
PP(int type;)								/* data type */
PP(int ar;)									/* base register */
PP(int32_t off;)							/* displacement */
PP(int xr;)									/* index register */
PP(int xt;)									/* index register type */
{
	register struct tnode *xp;

	xp = talloc(sizeof(struct tnode) - sizeof(union tval) + sizeof(struct xxsym));
	xp->t_op = SYMBOL;
	xp->t_type = type;
	xp->t_su = SU_ADDR;
	xp->t_ssp = 0;
	xp->t_sc = INDEXED;
	xp->t_offset = off;
	xp->t_reg = ar;
	xp->t_xreg = xr;
	xp->t_xtype = xt;
	return xp;
}


/* symcopy - copy symbol */
static VOID symcopy(P(const char *) from, P(char *) to)
PP(const char *from;)						/* from symbol */
PP(char *to;)								/* to symbol */
{
	register const char *p;
	register char *q;
	register short i;

	for (p = from, q = to, i = SSIZE; --i >= 0;)
	{
		*q++ = (*p ? *p++ : '\0');
	}
}


/* tcopy - expression tree copy */
struct tnode *tcopy(P(struct tnode *) tp, P(int) autof)
PP(struct tnode *tp;)
PP(int autof;)								/* {A_DOPRE,A_DOPOST} */
{
	register struct tnode *p;
	register short op;

	op = tp->t_op;
	switch (op)
	{
	case SYMBOL:
		if (tp->t_sc == EXTERNAL || tp->t_sc == EXTOFF)
		{
			p = cenalloc(tp->t_type, tp->t_sc, tp->t_symbol);
		} else
		{
			p = snalloc(tp->t_type, tp->t_sc, tp->t_offset, 0, 0);
			p->t_label = tp->t_label;
		}
		p->t_offset = tp->t_offset;
		p->t_reg = tp->t_reg;
		return p;

	case CINT:
		return cnalloc(tp->t_type, tp->t_value);

	case CLONG:
		return lcnalloc(tp->t_type, tp->t_lvalue);

	case CFLOAT:
		return fpcnalloc(tp->t_type, tp->t_lvalue);

	case DCLONG:
		p = lcnalloc(tp->t_type, tp->t_lvalue);
		p->t_op = DCLONG;
		return p;

	case AUTOINC:
	case AUTODEC:
		p = snalloc(tp->t_type, AUTO, 0L, 0, 0);
		if (op == AUTODEC)
		{
			if ((autof & A_DOPRE) != 0)
				p->t_op = AUTODEC;
		} else if (op == AUTOINC)
		{
			if ((autof & A_DOPOST) != 0)
				p->t_op = AUTOINC;
		}
		p->t_reg = tp->t_reg;
		return p;

	case POSTINC:
	case POSTDEC:
		if ((autof & A_DOPOST) == 0)
			return tcopy(tp->t_left, autof);
		goto copyop;
	case PREINC:
	case PREDEC:
		if ((autof & A_DOPRE) == 0)
			return tcopy(tp->t_left, autof);
		/* fall through */
	  copyop:
	default:
		if (ISASGOP(op))
		{
			if ((autof & A_DOPRE) == 0)
				return tcopy(tp->t_left, autof);
			autof |= A_DOPOST;			/* We'll come this way only once */
		}
		p = tnalloc(op, tp->t_type, 0, 0, &null, &null);
		if (BINOP(op))
			p->t_right = tcopy(tp->t_right, autof);
		p->t_left = tcopy(tp->t_left, autof);
		return p;
	}
}


/*
 * outlval - output long value
 * Output as hex with $ prefix.
 */
static VOID outlval(P(int32_t) lval)
PP(int32_t lval;)
{
	char digs[8];
	register short i, c;

	i = 0;
	do
	{
		digs[i++] = lval & 0xf;
		lval >>= 4;
		lval &= 0xfffffff;
	} while (lval);
	oputchar('$');
	while (--i >= 0)
	{
		c = digs[i];
		oputchar(c >= 10 ? c + ('a' - 10) : c + '0');
	}
}


/*
 * outaexpr - output address expression
 *
 * Z8002 addressing modes:
 *   Register:       R%d
 *   Register pair:  RR%d (for long)
 *   Immediate:      #value
 *   Direct:         address
 *   Base+offset:    offset(R%d)    [REGOFF]
 *   Indexed:        offset(R%d)    [simplified - Z8002 doesn't have 68000-style indexed]
 */
VOID outaexpr(P(struct tnode *) tp, P(int) flags)
PP(struct tnode *tp;)
PP(int flags;)								/* flags (IMMED,LOFFSET,...) */
{
	register short reg, lab;
	register int32_t off;
	int32_t l;

	if (tp->t_op == ADDR)
	{
		tp = tp->t_left;
		oputchar('#');
	}
	off = tp->t_offset;
	reg = tp->t_reg;
	lab = tp->t_label;
	switch (tp->t_op)
	{
	case AUTOINC:
		/* Z8002: no auto-increment addressing mode — just use indirect */
		oprintf("@R%d", reg);
		break;

	case AUTODEC:
		/* Z8002: no auto-decrement addressing mode — just use indirect */
		oprintf("@R%d", reg);
		break;

	case CINT:
		if ((flags & A_DOIMMED) != 0)
			oputchar('#');
		oprintf("%d", tp->t_value);
		break;

	case DCLONG:
	case CLONG:
	case CFLOAT:
		if ((flags & A_DOIMMED) != 0)
			oputchar('#');
		outlval(tp->t_lvalue);
		break;

	case SYMBOL:
		if (off)
		{
			switch (tp->t_sc)
			{
			default:
				oprintf("%ld+", (long)off);
				break;

			case REGOFF:
				/* offset will be combined with register below */
				break;

			case CINDR:
			case CLINDR:
			case CFINDR:
			case INDEXED:
				break;

			case REGISTER:
				error("invalid register expression");
				break;
			}
		}
		switch (tp->t_sc)
		{
		case REGISTER:
			/*
			 * Z8002: type-aware register output.
			 * LONG/FLOAT use register pairs RR%d.
			 * CHAR uses byte register RL%d.
			 * Others use R%d.
			 */
			if (LONGTYPE(tp->t_type))
				oprintf("RR%d", reg & ~1);
			else if (tp->t_type == CHAR || tp->t_type == UCHAR)
				oprintf("RL%d", reg);
			else
				oprintf("R%d", reg);
			break;

		case REGOFF:
			/* Z8002: base+displacement: offset(Rn) */
			if (off)
				oprintf("%ld(R%d)", (long)off, reg);
			else
				oprintf("@R%d", reg);
			break;

		case EXTERNAL:
			oprintf("_%.*s", SSIZE, tp->t_symbol);
			break;

		case EXTOFF:
			/* Z8002: external+register offset: _sym(Rn) */
			oprintf("_%.*s(R%d)", SSIZE, tp->t_symbol, reg);
			break;

		case STATIC:
			oprintf("L%d", lab);
			break;

		case STATOFF:
			oprintf("L%d(R%d)", lab, reg);
			break;

		case INDEXED:
			/* Z8002: base+index — use offset(Rn) for base, index handled separately */
			oprintf("%ld(R%d)", (long)off, reg);
			break;

		case CINDR:
			oprintf("%ld", (long)off);
			break;

		case CLINDR:
		case CFINDR:
			l = tp->t_offset;
			outlval(l);
			break;

		default:
			error("invalid storage class %d\n", tp->t_sc);
			break;
		}
		break;

	default:
		error("invalid operator %s\n", opname[tp->t_op]);
		break;
	}
}


/*
 * outtype - output Z8002 type suffix
 *
 * Z8002 uses mnemonic suffixes (no dot):
 *   CHAR → "b"  (byte operations: ldb, addb, etc.)
 *   LONG/FLOAT → "l"  (long operations: ldl, addl, etc.)
 *   INT/pointer → nothing (word is default)
 *
 * Note: pointers are 16-bit on Z8002, so they use word operations.
 */
VOID outtype(P(int) type)
PP(int type;)
{
	if (LONGTYPE(type))
		oprintf("l");
	else if (type == CHAR || type == UCHAR)
		oprintf("b");
	/* word (INT, UNSIGNED, pointers) = no suffix */
}


/*
 * outatype - output address type suffix
 *
 * Z8002: Only emit "l" for actual LONG/FLOAT types.
 * Pointers are 16-bit — no suffix needed.
 */
VOID outatype(P(int) type)
PP(int type;)
{
	if (LONGTYPE(type))
		oprintf("l");
}


/* outextend - output register type conversion */
VOID outextend(P(struct tnode *) tp, P(int) type, P(int) reg)
PP(struct tnode *tp;)
PP(int type;)								/* type to convert to */
PP(int reg;)								/* register to convert */
{
	int from = tp->t_type;

	/* CHAR/UCHAR → INT/UNSIGNED (byte → word) */
	if ((from == CHAR || from == UCHAR) && !LONGTYPE(type) && from != type) {
		if (from == CHAR)
			oprintf("\textsb R%d\n", reg);	/* sign-extend low byte */
		else
			oprintf("\tand R%d,#255\n", reg);	/* zero-extend (mask high byte) */
		from = (from == CHAR) ? INT : UNSIGNED;
	}

	/* INT/UNSIGNED → LONG (word → long) */
	if (!LONGTYPE(from) && LONGTYPE(type))
	{
		if (UNSIGN(from) || from == UCHAR)
			OUTUEXT(reg);
		else
			OUTEXT(reg);
	}
	/* LONG → INT truncation — extract low word from pair */
	else if (LONGTYPE(from) && !LONGTYPE(type))
	{
		oprintf("\tld R%d,R%d\n", reg, reg | 1);
	}
}


/*
 * outrr - output register to register instruction
 *
 * Z8002 operand order: op dst,src → op R2,R1
 * But for some instructions the operand order matters differently.
 * For outrr, r1=source, r2=destination (matching the calling convention).
 *
 * Z8002 type-aware register names: R%d, RR%d, RL%d/RH%d
 */
VOID outrr(P(const char *) ins, P(int) r1, P(int) r2, P(struct tnode *) tp)
PP(const char *ins;)
PP(int r1;)
PP(int r2;)
PP(struct tnode *tp;)
{
	oputchar('\t');
	oprintf("%s", ins);
	outtype(tp->t_type);
	/* Z8002: destination first: op dst,src → op R2,R1 */
	if (LONGTYPE(tp->t_type))
		oprintf(" RR%d,RR%d\n", r2 & ~1, r1 & ~1);
	else if (tp->t_type == CHAR || tp->t_type == UCHAR)
		oprintf(" RL%d,RL%d\n", r2, r1);
	else
		oprintf(" R%d,R%d\n", r2, r1);
}


/*
 * outmovr - output "ld R2,R1" instruction (move r1 to r2)
 *
 * Z8002: ld dst,src
 */
VOID outmovr(P(int) r1, P(int) r2, P(struct tnode *) tp)
PP(int r1;)
PP(int r2;)
PP(struct tnode *tp;)
{
	if (r1 != r2)
		outrr("ld", r1, r2, tp);
}


/*
 * outcmpm - output memory compare
 *
 * Z8002 has CPIR/CPDR for block compare — but for now,
 * use explicit load-and-compare sequence.
 */
VOID outcmpm(P(struct tnode *) tp)
PP(struct tnode *tp;)
{
	int lreg = tp->t_left->t_reg;
	int rreg = tp->t_right->t_reg;
	int type = tp->t_left->t_type;
	int sz;

	/* Load from right into R0, compare with left */
	oprintf("\tld");
	outtype(type);
	oprintf(" R0,@R%d\n", rreg);
	oprintf("\tcp");
	outtype(type);
	oprintf(" R0,@R%d\n", lreg);

	/* Advance pointers */
	if (LONGTYPE(type))
		sz = 4;
	else if (type == CHAR || type == UCHAR)
		sz = 1;
	else
		sz = 2;
	oprintf("\tinc R%d,#%d\n\tinc R%d,#%d\n", lreg, sz, rreg, sz);
}


/*
 * outcreg - output reference to compiler temp register
 *
 * Z8002: type-aware register output.
 * LONG/FLOAT use register pairs RR%d.
 * CHAR uses byte register RL%d.
 * Others use R%d.
 */
VOID outcreg(P(int) reg, P(int) type)
PP(int reg;)
PP(int type;)
{
	if (reg > HICREG)
		error("expression too complex");
	if (LONGTYPE(type))
		oprintf("RR%d", reg & ~1);
	else if (type == CHAR || type == UCHAR)
		oprintf("RL%d", reg);
	else
		oprintf("R%d", reg);
}


/*
 * outcmp0 - output compare with zero
 *
 * Z8002 has no "tst" instruction — use "cp Rn,#0" or "testl RRn" if available.
 * For simplicity, always use cp/cpl with #0.
 */
VOID outcmp0(P(int) reg, P(struct tnode *) tp)
PP(int reg;)
PP(struct tnode *tp;)
{
	if (LONGTYPE(tp->t_type))
		oprintf("\tcpl RR%d,#0\n", reg & ~1);
	else if (tp->t_type == CHAR || tp->t_type == UCHAR)
		oprintf("\tcpb RL%d,#0\n", reg);
	else
		oprintf("\tcp R%d,#0\n", reg);
}


/*
 * outrpush - output "push @R15,Rn" / "pushl @R15,RRn"
 *
 * Z8002: PUSH @R15,src (destination is always @R15 = stack)
 * pflag: 1 = push (pre-decrement SP), 0 = store at (SP) without push
 */
VOID outrpush(P(int) reg, P(struct tnode *) tp, P(int) pflag)
PP(int reg;)
PP(struct tnode *tp;)
PP(int pflag;)
{
	if (pflag)
	{
		/* Real push */
		if (LONGTYPE(tp->t_type))
			oprintf("\tpushl @R15,RR%d\n", reg & ~1);
		else
			oprintf("\tpush @R15,R%d\n", reg);
	} else
	{
		/* Store at current SP (FORSP) */
		if (LONGTYPE(tp->t_type))
			oprintf("\tldl @R15,RR%d\n", reg & ~1);
		else
			oprintf("\tld @R15,R%d\n", reg);
	}
}


/*
 * outdbra - output decrement-and-branch (Z8002: DJNZ / DBJNZ)
 *
 * Z8002 has DJNZ Rn,displacement for decrement-and-branch-if-nonzero.
 * This is used for loop optimization.
 */
int outdbra(P(int) dir, P(int) op, P(struct tnode *) ltp, P(struct tnode *) rtp, P(int) lab)
PP(int dir;)
PP(int op;)
PP(struct tnode *ltp;)
PP(struct tnode *rtp;)
PP(int lab;)
{
	if (!((dir != 0 && op == NEQUALS) || (dir == 0 && op == EQUALS)))
		return 0;
	if (!(ltp->t_op == PREDEC && rtp->t_op == CINT && rtp->t_value == -1 &&
		  ltp->t_left->t_type == INT && ltp->t_left->t_sc == REGISTER))
		return 0;

	oprintf("\tdjnz R%d,L%d\n", ltp->t_left->t_reg, lab);
	return 1;
}


/*
 * cenalloc - code generator external node allocation
 * This may be coalesced into enalloc in parser.
 */
struct tnode *cenalloc(P(int) type, P(int) sc, P(const char *) sym)
PP(int type;)								/* type of symbol */
PP(int sc;)									/* storage class */
PP(const char *sym;)						/* symbol name */
{
	register struct tnode *ep;

	ep = talloc(sizeof(struct tnode) - sizeof(union tval) + sizeof(struct xesym));
	ep->t_op = SYMBOL;
	ep->t_type = type;
	ep->t_sc = sc;
	ep->t_su = 0;
	ep->t_offset = 0;
	symcopy(sym, ep->t_symbol);
	return ep;
}


/*
 * popstack - clear off the stack after a call if necessary
 *
 * Z8002: inc R15,#nb (stack grows downward, R15 is SP)
 * INC range is 1-16, so for larger values use ADD.
 */
VOID popstack(P(int) nb)
PP(int nb;)
{
	if (nb > 0 && nb <= 16)
		oprintf("\tinc R15,#%d\n", nb);
	else if (nb > 0)
		oprintf("\tadd R15,#%d\n", nb);
}
