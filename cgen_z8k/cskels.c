#include "cgen.h"

/*
 * Z8002 Code Skeletons
 *
 * Key Z8002 differences from 68000:
 * - Operand order reversed: Z8002 is "op dst,src" not "op src,dst"
 * - Indirect patterns use S_NEXT|S_INDR + NAR instead of S_INDR + CAR
 *   because Z8002 has unified registers (no D-reg/A-reg split)
 * - PSH is a PREFIX macro: emits "push @R15," then source follows
 * - CLR, EXTW, EXTL are unary — no operand reversal needed
 */

/* empty_skel no longer needed — all skeleton groups are implemented */


/* ================================================================
 * LOAD skeleton group (fr_ld=28, fs_ld=11)
 * ================================================================ */

/* ctlod00: load zero char — clr Rn */
static char const ctlod00[] = {
	CLR, ' ', CR, '\n',
	0
};

/* ctlod01: load zero int/long — clr[l] Rn/RRn */
static char const ctlod01[] = {
	CLR, TLEFT, ' ', CR, '\n',
	0
};

/* ct11d: load unsigned char — clr Rn; ldb CR,src */
static char const ct11d[] = {
	CLR, ' ', CR, '\n',
	MOV, TLEFT, ' ', CR, ',', LADDR, '\n',
	0
};

/* ctlod02: load signed char — ldb CR,src; extsb CR */
static char const ctlod02[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '\n',
	EXTW, ' ', CR, '\n',
	0
};

/* ctlod03: load int/long/float/pointer — ld CR,src */
static char const ctlod03[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '\n',
	0
};

/* ct14d: load unsigned char via indirect */
static char const ct14d[] = {
	LEFT, S_NEXT | S_INDR,
	CLR, ' ', CR, '\n',
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	0
};

/* ctlod04: load signed char via indirect */
static char const ctlod04[] = {
	LEFT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	EXTW, ' ', CR, '\n',
	0
};

/* ctlod05: load int/long via indirect */
static char const ctlod05[] = {
	LEFT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	0
};

static struct skeleton const fr_ld[] = {
	{ SU_ZERO | T_CHAR, SU_ANY | T_ANY, ctlod00 },
	{ SU_ZERO | T_INT, SU_ANY | T_ANY, ctlod01 },
	{ SU_ZERO | T_LONG, SU_ANY | T_ANY, ctlod01 },
	{ SU_ADDR | T_UCHAR, SU_ANY | T_ANY, ct11d },
	{ SU_ADDR | T_CHAR, SU_ANY | T_ANY, ctlod02 },
	{ SU_ADDR | T_ANY, SU_ANY | T_ANY, ctlod03 },
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, ctlod03 },
	{ SU_ADDR | T_FLOAT, SU_ANY | T_ANY, ctlod03 },
	{ SU_ANY | T_UCHAR | T_INDR, SU_ANY | T_ANY, ct14d },
	{ SU_ANY | T_CHAR | T_INDR, SU_ANY | T_ANY, ctlod04 },
	{ SU_ANY | T_INT | T_INDR, SU_ANY | T_ANY, ctlod05 },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_ANY, ctlod05 },
	{ SU_ANY | T_FLOAT | T_INDR, SU_ANY | T_ANY, ctlod05 },
	{ 0, 0, NULL }
};


/* fs_ld: load onto stack (PSH = prefix emitting "push @R15,") */

static char const ct17d[] = { PSH, '#', '0', '\n', 0 };
static char const ctlod06[] = { PSH, '#', '0', '\n', 0 };
static char const ctlod07[] = { LEFT, 0, PSH, CR, '\n', 0 };
static char const ctlod08[] = { PSH, LADDR, '\n', 0 };
static char const ctlod09[] = {
	LEFT, S_NEXT | S_INDR,
	PSH, LOFFSET, '(', NAR, ')', '\n',
	0
};

static struct skeleton const fs_ld[] = {
	{ SU_ZERO | T_CHAR, SU_ANY | T_ANY, ct17d },
	{ SU_ZERO | T_INT, SU_ANY | T_ANY, ctlod06 },
	{ SU_ZERO | T_LONG, SU_ANY | T_ANY, ctlod06 },
	{ SU_ADDR | T_CHAR, SU_ANY | T_ANY, ctlod07 },
	{ SU_ADDR | T_INT, SU_ANY | T_ANY, ctlod08 },
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, ctlod08 },
	{ SU_ADDR | T_FLOAT, SU_ANY | T_ANY, ctlod08 },
	{ SU_ANY | T_INT | T_INDR, SU_ANY | T_ANY, ctlod09 },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_ANY, ctlod09 },
	{ SU_ANY | T_FLOAT | T_INDR, SU_ANY | T_ANY, ctlod09 },
	{ 0, 0, NULL }
};


/* ================================================================
 * ASSIGN skeleton group (fr_assign=20, fe_assign=2)
 *
 * fr_assign: assignment returning value in register (FORREG)
 * fe_assign: assignment for effect only (FOREFF)
 *
 * For ASSIGN, OP expands to "ld" (I_LD), AOP to "clr" (I_CLR).
 * 68k pattern: OP TLEFT ' ' src ',' dst
 * Z8002 pattern: OP TLEFT ' ' dst ',' src (operands reversed)
 * ================================================================ */


/* --- fr_assign (FORREG): assign and return value in register --- */

/*
 * ctasg02: generic assign: compile right to reg, store to dst
 * 68k: [right→reg] EXRL; move.l Dn,dst
 * Z8002: [right→reg] EXRL; ld dst,Rn (operands reversed)
 */
static char const ctasg02[] = {
	RIGHT, 0,
	EXRL,
	OP, TLEFT, ' ', LADDR, ',', CR, '\n',
	0
};

/*
 * ctasg03: long indr dst = unsigned int src (zero-extend then store via ptr)
 * 68k: [left→INDR] clr.l Dn; move.w src,Dn; move.l off(An),Dn; move.l Dn,off(An)
 * Z8002: uses S_NEXT|S_INDR for address, NAR for address reg
 */
static char const ctasg03[] = {
	LEFT, S_NEXT | S_INDR,
	CLRL, ' ', CR, '\n',
	MOV, TRIGHT, ' ', CR, ',', RADDR, '\n',
	OP, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	0
};

/*
 * ctasg04: indr dst = addressable src (compile left INDR, load src, store)
 * 68k: [left→INDR] move.w src,Dn; EXRL; move.l Dn,off(An)
 * Z8002: S_NEXT|S_INDR for address
 */
static char const ctasg04[] = {
	LEFT, S_NEXT | S_INDR,
	MOV, TRIGHT, ' ', CR, ',', RADDR, '\n',
	EXRL,
	OP, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/*
 * ctasg05: easy indr dst = easy src (right to reg, left INDR to nreg, store)
 * 68k: [right→reg] EXRL; [left→INDR nreg] move.l Dn,off(An)
 * Z8002: S_NEXT|S_INDR for address
 */
static char const ctasg05z[] = {
	RIGHT, 0,
	EXRL,
	LEFT, S_NEXT | S_INDR,
	OP, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/*
 * ctasg06: hard indr dst = hard src (push addr, compile right, pop addr, store)
 * 68k: [left→stack INDR] [right→reg] EXRL; move.l (sp)+,An; move.l Dn,off(An)
 * Z8002: push address to stack, compile right, pop address back
 * Note: POP on Z8002 outputs @R15 — need to load into nreg then adjust stack
 */
static char const ctasg06z[] = {
	LEFT, S_STACK | S_INDR,
	RIGHT, 0,
	EXRL,
	MOVL, ' ', NR, ',', POP, '\n',
	OP, TLEFT, ' ', LOFFSET, '(', NR, ')', ',', CR, '\n',
	0
};

static struct skeleton const fr_assign[] = {
	{ SU_ADDR | T_LONG, SU_ADDR | T_UNSN, ctasg02 },
	{ SU_ADDR | T_ANY, SU_ANY | T_ANY, ctasg02 },
	{ SU_ADDR | T_ANY, SU_ANY | T_LONG, ctasg02 },
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, ctasg02 },
	{ SU_ADDR | T_LONG, SU_ANY | T_LONG, ctasg02 },
	{ SU_ADDR | T_FLOAT, SU_ANY | T_FLOAT, ctasg02 },
	{ SU_ANY | T_LONG | T_INDR, SU_ADDR | T_UNSN, ctasg03 },
	{ SU_ANY | T_ANY | T_INDR, SU_ADDR | T_INT, ctasg04 },
	{ SU_ANY | T_ANY | T_INDR, SU_ADDR | T_LONG, ctasg04 },
	{ SU_ANY | T_LONG | T_INDR, SU_ADDR | T_INT, ctasg04 },
	{ SU_ANY | T_LONG | T_INDR, SU_ADDR | T_LONG, ctasg04 },
	{ SU_ANY | T_FLOAT | T_INDR, SU_ADDR | T_FLOAT, ctasg04 },
	{ SU_EASY | T_ANY | T_INDR, SU_EASY | T_ANY, ctasg05z },
	{ SU_EASY | T_ANY | T_INDR, SU_EASY | T_LONG, ctasg05z },
	{ SU_EASY | T_LONG | T_INDR, SU_EASY | T_LONG, ctasg05z },
	{ SU_EASY | T_LONG | T_INDR, SU_EASY | T_ANY, ctasg05z },
	{ SU_ANY | T_ANY | T_INDR, SU_ANY | T_ANY, ctasg06z },
	{ SU_ANY | T_ANY | T_INDR, SU_ANY | T_LONG, ctasg06z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_ANY, ctasg06z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_LONG, ctasg06z },
	{ SU_ANY | T_FLOAT | T_INDR, SU_ANY | T_FLOAT, ctasg06z },
	{ 0, 0, NULL }
};


/* --- fe_assign (FOREFF): assign for effect only, no return value --- */

/*
 * ctasg07: clear addressed dst (AOP = clr)
 * 68k: clr.w dst / clr.l dst
 * Z8002: clr dst / clrl dst (unary — no reversal)
 */
static char const ctasg07[] = {
	AOP, TLEFT, ' ', LADDR, '\n',
	0
};

/*
 * ctasg08: clear indirect dst
 * Z8002: S_NEXT|S_INDR for address
 */
static char const ctasg08[] = {
	LEFT, S_NEXT | S_INDR,
	AOP, TLEFT, ' ', LOFFSET, '(', NAR, ')', '\n',
	0
};

/*
 * ctasg09: store addressable src to addressable dst
 * 68k: move.w src,dst / move.l src,dst
 * Z8002: ld dst,src (operands reversed)
 */
static char const ctasg09[] = {
	OP, TLEFT, ' ', LADDR, ',', RADDR, '\n',
	0
};

/*
 * ctasg_indr_convz: indirect dst = cross-type src (via reg + extend)
 * Z8002: compile left INDR, compile right to reg, extend, store
 */
static char const ctasg_indr_convz[] = {
	LEFT, S_NEXT | S_INDR,
	RIGHT, 0,
	EXRL,
	OP, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/*
 * ctasg17: indr src → addressed dst
 * 68k: [right→INDR] move.w off(An),dst
 * Z8002: [right→INDR nreg] ld dst,off(Rn)
 */
static char const ctasg17z[] = {
	RIGHT, S_NEXT | S_INDR,
	OP, TLEFT, ' ', LADDR, ',', ROFFSET, '(', NAR, ')', '\n',
	0
};

/*
 * ctasg18: generic: compile right to reg, extend, store to addressed dst
 * 68k: [right→reg] EXRL; move.l Dn,dst
 * Z8002: [right→reg] EXRL; ld dst,Rn
 */
static char const ctasg18[] = {
	RIGHT, 0,
	EXRL,
	OP, TLEFT, ' ', LADDR, ',', CR, '\n',
	0
};

/*
 * ctasg19: addressed src → indr dst
 * 68k: [left→INDR] move.w src,off(An)
 * Z8002: S_NEXT|S_INDR for address; ld off(Rn),src
 */
static char const ctasg19z[] = {
	LEFT, S_NEXT | S_INDR,
	OP, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', RADDR, '\n',
	0
};

/*
 * ctasg20: indr src → indr dst (both easy)
 * 68k: [left→INDR] [right→INDR nreg] move.w off(NAR),off(CAR)
 * Z8002: two indirects — need different regs
 * On Z8002: LEFT to INDR in nreg, RIGHT to INDR in nreg+1
 * But we only have S_INDR and S_NEXT flags... use S_NEXT|S_INDR for left
 * For RIGHT, use S_NEXT|S_INDR — but that would overwrite nreg.
 * Actually the 68000 original uses LEFT S_INDR (→CAR=freg), RIGHT S_NEXT|S_INDR (→NAR=nreg)
 * For Z8002 we must swap: RIGHT S_NEXT|S_INDR (→NAR=nreg), LEFT S_INDR... no.
 * Actually: just do it through a register. Load from right, store to left.
 */
static char const ctasg20z[] = {
	RIGHT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', ROFFSET, '(', NAR, ')', '\n',
	LEFT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/*
 * ctasg23: hard indr dst = any src (push right to stack, compile left INDR, pop and store)
 * 68k: [right→stack] [left→INDR] move.w (sp)+,off(An)
 * Z8002: push right, compile left addr, pop into NAR temp, ld to dest
 * POP outputs @R15, then we need inc R15 for the stack cleanup
 */
static char const ctasg23z[] = {
	RIGHT, S_STACK,
	LEFT, S_NEXT | S_INDR,
	OP, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', POP, '\n',
	0
};

static struct skeleton const fe_assign[] = {
	{ SU_ADDR | T_ANY, SU_ZERO | T_ANY, ctasg07 },
	{ SU_ADDR | T_LONG, SU_ZERO | T_ANY, ctasg07 },
	{ SU_ANY | T_ANY | T_INDR, SU_ZERO | T_ANY, ctasg08 },
	{ SU_ADDR | T_CHAR, SU_ADDR | T_CHAR, ctasg09 },
	{ SU_ADDR | T_INT, SU_ADDR | T_INT, ctasg09 },
	{ SU_ADDR | T_LONG, SU_ADDR | T_LONG, ctasg09 },
	{ SU_ADDR | T_ANY, SU_CONST | T_INT, ctasg09 },
	{ SU_ADDR | T_LONG, SU_CONST | T_INT, ctasg09 },
	{ SU_ADDR | T_ANY, SU_REG | T_INT, ctasg09 },
	{ SU_ADDR | T_ANY, SU_REG | T_LONG, ctasg09 },
	{ SU_ADDR | T_FLOAT, SU_ADDR | T_FLOAT, ctasg09 },
	/* Z8002: cross-type assignments via generic RIGHT+EXRL path */
	{ SU_REG | T_LONG, SU_ADDR | T_UCHAR, ctasg18 },
	{ SU_REG | T_ANY, SU_ADDR | T_UCHAR, ctasg18 },
	{ SU_REG | T_LONG, SU_ADDR | T_CHAR, ctasg18 },
	{ SU_REG | T_ANY, SU_ADDR | T_CHAR, ctasg18 },
	{ SU_REG | T_LONG, SU_ADDR | T_UNSN, ctasg18 },
	{ SU_REG | T_LONG, SU_ADDR | T_UCHAR, ctasg18 },
	{ SU_REG | T_INT, SU_ADDR | T_UCHAR, ctasg18 },
	{ SU_REG | T_INT, SU_ADDR | T_CHAR, ctasg18 },
	{ SU_REG | T_LONG, SU_ADDR | T_CHAR, ctasg18 },
	{ SU_REG | T_LONG, SU_ADDR | T_UCHAR, ctasg18 },
	{ SU_REG | T_LONG, SU_ADDR | T_INT, ctasg18 },
	{ SU_REG | T_LONG, SU_ADDR | T_INT, ctasg18 },
	{ SU_ADDR | T_LONG, SU_ADDR | T_UNSN, ctasg18 },
	{ SU_ADDR | T_LONG, SU_ADDR | T_INT, ctasg18 },
	{ SU_ANY | T_LONG | T_INDR, SU_ADDR | T_UNSN, ctasg_indr_convz },
	{ SU_ADDR | T_CHAR, SU_ANY | T_CHAR | T_INDR, ctasg17z },
	{ SU_ADDR | T_INT, SU_ANY | T_INT | T_INDR, ctasg17z },
	{ SU_ADDR | T_LONG, SU_ANY | T_LONG | T_INDR, ctasg17z },
	{ SU_ADDR | T_ANY, SU_ANY | T_ANY, ctasg18 },
	{ SU_ADDR | T_ANY, SU_ANY | T_LONG, ctasg18 },
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, ctasg18 },
	{ SU_ADDR | T_LONG, SU_ANY | T_LONG, ctasg18 },
	{ SU_ANY | T_INT | T_INDR, SU_ADDR | T_INT, ctasg19z },
	{ SU_ANY | T_CHAR | T_INDR, SU_ADDR | T_CHAR, ctasg19z },
	{ SU_ANY | T_LONG | T_INDR, SU_ADDR | T_LONG, ctasg19z },
	{ SU_ANY | T_ANY | T_INDR, SU_CONST | T_INT, ctasg19z },
	{ SU_ANY | T_LONG | T_INDR, SU_CONST | T_INT, ctasg19z },
	{ SU_ANY | T_FLOAT | T_INDR, SU_ADDR | T_FLOAT, ctasg19z },
	{ SU_EASY | T_CHAR | T_INDR, SU_EASY | T_CHAR | T_INDR, ctasg20z },
	{ SU_EASY | T_INT | T_INDR, SU_EASY | T_INT | T_INDR, ctasg20z },
	{ SU_EASY | T_LONG | T_INDR, SU_EASY | T_LONG | T_INDR, ctasg20z },
	{ SU_EASY | T_ANY | T_INDR, SU_EASY | T_ANY, ctasg05z },
	{ SU_EASY | T_ANY | T_INDR, SU_EASY | T_LONG, ctasg05z },
	{ SU_EASY | T_LONG | T_INDR, SU_EASY | T_ANY, ctasg05z },
	{ SU_EASY | T_LONG | T_INDR, SU_EASY | T_LONG, ctasg05z },
	{ SU_ANY | T_INT | T_INDR, SU_ANY | T_ANY, ctasg23z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_LONG, ctasg23z },
	{ SU_ANY | T_ANY | T_INDR, SU_ANY | T_ANY, ctasg06z },
	{ SU_ANY | T_ANY | T_INDR, SU_ANY | T_LONG, ctasg06z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_ANY, ctasg06z },
	{ SU_ANY | T_FLOAT | T_INDR, SU_ADDR | T_FLOAT, ctasg06z },
	{ 0, 0, NULL }
};


/* ================================================================
 * ADD/SUB/AND/OR skeleton group (fr_add=12, fs_op=9, fc_fix=6)
 *
 * fr_add: binary arithmetic/logic into register (FORREG)
 * fs_op: binary arithmetic/logic onto stack (FORSTACK)
 * fc_fix: condition code fixup (FORCC)
 *
 * For ADD/SUB/AND/OR, OP expands to the appropriate mnemonic.
 * 68k: OP TLEFT ' ' src ',' CR (i.e., add.w src,Dn)
 * Z8002: OP TLEFT ' ' CR ',' src (i.e., add Rn,src — reversed)
 * ================================================================ */

/* ctbop00: left op zero (char) — just compile left, clear result */
static char const ctbop00[] = {
	LEFT, 0,
	CLR, ' ', CR, '\n',
	0
};

/* ctbop01: left op zero (int/long) — compile left, clear result */
static char const ctbop01[] = {
	LEFT, 0,
	CLR, TLEFT, ' ', CR, '\n',
	0
};

/*
 * ctbop02: left op addressed right (int/long)
 * 68k: [left→reg] EXLR; add.w src,Dn
 * Z8002: [left→reg] EXLR; add Rn,src (reversed)
 */
static char const ctbop02[] = {
	LEFT, 0,
	EXLR,
	OP, TRIGHT, ' ', CR, ',', RADDR, '\n',
	0
};

/*
 * ctbop03: long left op unsigned int right (zero-extend right)
 * 68k: [left→reg] clr.l NR; move.w src,NR; add.l NR,CR
 * Z8002: [left→reg] clrl NR; ld NR,src; addl CR,NR (reversed)
 */
static char const ctbop03[] = {
	LEFT, 0,
	CLRL, ' ', NR, '\n',
	MOV, TRIGHT, ' ', NR, ',', RADDR, '\n',
	OP, TLEFT, ' ', CR, ',', NR, '\n',
	0
};

/*
 * ctbop04: unsigned int left op long right (zero-extend left)
 * 68k: clr.l CR; move.w src,CR; [right→NR] add.w NR,CR
 * Z8002: clrl CR; ld CR,src; [right→NR] add CR,NR (reversed)
 */
static char const ctbop04[] = {
	CLRL, ' ', CR, '\n',
	MOV, TLEFT, ' ', CR, ',', LADDR, '\n',
	RIGHT, S_NEXT,
	OP, TRIGHT, ' ', CR, ',', NR, '\n',
	0
};

/*
 * ctbop05: left op addressed right (long, or const int)
 * 68k: [left→reg] add.l src,CR
 * Z8002: [left→reg] addl CR,src (reversed)
 */
static char const ctbop05[] = {
	LEFT, 0,
	OP, TLEFT, ' ', CR, ',', RADDR, '\n',
	0
};

/*
 * ctbop06: left op easy right (both need compilation)
 * 68k: [left→reg] EXLR; [right→NR] EXRLN; add.w NR,CR
 * Z8002: reversed: add CR,NR
 */
static char const ctbop06[] = {
	LEFT, 0,
	EXLR,
	RIGHT, S_NEXT,
	EXRLN,
	OP, TEITHER, ' ', CR, ',', NR, '\n',
	0
};

/*
 * ctbop07: long left op any right (push right, then add)
 * 68k: [right→reg] EXRL; move.l CR,-(sp); [left→reg] add.l (sp)+,CR
 * Z8002: [right→reg] EXRL; push right; [left→reg]; add CR,@R15; pop
 */
static char const ctbop07[] = {
	RIGHT, 0,
	EXRL,
	PSH, CR, '\n',
	LEFT, 0,
	OP, TLEFT, ' ', CR, ',', POP, '\n',
	0
};

/*
 * ctbop08: any left op long right (push right to stack)
 * 68k: [right→stack] [left→reg] EXLR; add.w (sp)+,CR
 * Z8002: [right→stack] [left→reg] EXLR; add CR,@R15; pop
 */
static char const ctbop08[] = {
	RIGHT, S_STACK,
	LEFT, 0,
	EXLR,
	OP, TRIGHT, ' ', CR, ',', POP, '\n',
	0
};

/*
 * ctbop09: any left op any right (general case via stack)
 * 68k: [right→stack] [left→reg] add.w (sp)+,CR
 * Z8002: [right→stack] [left→reg] add CR,@R15; pop
 */
static char const ctbop09[] = {
	RIGHT, S_STACK,
	LEFT, 0,
	OP, ' ', CR, ',', POP, '\n',
	0
};

/*
 * ctbop12: float operation via library call
 * 68k: [right→stack] [left→stack] jsr _fpadd; pop 8
 * Z8002: same pattern
 */
static char const ctbop12[] = {
	RIGHT, S_STACK,
	LEFT, S_STACK,
	JSR, ' ', OPCALL, '\n',
	POP8, 0
};

static struct skeleton const fr_add[] = {
	{ SU_ANY | T_CHAR, SU_ZERO | T_ANY, ctbop00 },
	{ SU_ANY | T_INT, SU_ZERO | T_ANY, ctbop01 },
	{ SU_ANY | T_LONG, SU_ZERO | T_ANY, ctbop01 },
	{ SU_ANY | T_ANY, SU_ADDR | T_INT, ctbop02 },
	{ SU_ANY | T_ANY, SU_ADDR | T_LONG, ctbop02 },
	{ SU_ANY | T_LONG, SU_ADDR | T_LONG, ctbop02 },
	{ SU_ANY | T_LONG, SU_ADDR | T_UNSN, ctbop03 },
	{ SU_ADDR | T_UNSN, SU_ANY | T_LONG, ctbop04 },
	{ SU_ANY | T_LONG, SU_CONST | T_INT, ctbop05 },
	{ SU_ANY | T_LONG, SU_ADDR | T_LONG, ctbop05 },
	{ SU_ANY | T_ANY, SU_EASY | T_ANY, ctbop06 },
	{ SU_ANY | T_ANY, SU_EASY | T_LONG, ctbop06 },
	{ SU_ANY | T_LONG, SU_EASY | T_ANY, ctbop06 },
	{ SU_ANY | T_LONG, SU_EASY | T_LONG, ctbop06 },
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, ctbop07 },
	{ SU_ANY | T_ANY, SU_ANY | T_LONG, ctbop08 },
	{ SU_ANY | T_LONG, SU_ANY | T_LONG, ctbop08 },
	{ SU_ANY | T_ANY, SU_ANY | T_ANY, ctbop09 },
	{ SU_ANY | T_FLOAT, SU_ANY | T_FLOAT, ctbop12 },
	{ 0, 0, NULL }
};


/*
 * fs_op: binary op onto stack (FORSTACK)
 * 68k: [left→stack] add.l src,4(sp)
 * Z8002: [left→stack] add @R15,src (operates on top of stack)
 */
static char const ctbop10[] = {
	LEFT, S_STACK,
	OP, TLEFTL, ' ', STK, ',', RADDR, '\n',
	0
};

static char const ctbop11[] = {
	LEFT, S_STACK,
	RIGHT, 0,
	OP, TLEFTL, ' ', STK, ',', CR, '\n',
	0
};

static struct skeleton const fs_op[] = {
	{ SU_ANY | T_ANY, SU_CONST | T_INT, ctbop10 },
	{ SU_ANY | T_LONG, SU_CONST | T_INT, ctbop10 },
	{ SU_ANY | T_ANY, SU_ANY | T_ANY, ctbop11 },
	{ 0, 0, NULL }
};


/*
 * fc_fix: condition code fixup (FORCC)
 * Just compiles the entire tree — target-independent.
 */
static char const ctfix01[] = {
	TREE, 0,
	0
};

static struct skeleton const fc_fix[] = {
	{ SU_ANY | T_ANY, SU_ANY | T_ANY, ctfix01 },
	{ SU_ANY | T_LONG, SU_ANY | T_LONG, ctfix01 },
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, ctfix01 },
	{ SU_ANY | T_ANY, SU_ANY | T_LONG, ctfix01 },
	{ 0, 0, NULL }
};


/* ================================================================
 * COMPARE/BRANCH skeleton group (fc_rel=7)
 *
 * For comparisons, OP expands to "cp" (I_CP).
 * 68k: cmp.w src,Dn (sets CC from Dn - src)
 * Z8002: cp Rn,src (sets CC from Rn - src — same semantic, reversed syntax)
 *
 * Many skeletons are shared with fr_add and fe_assign.
 * ================================================================ */

/* ctrel01: compare with zero — just compile left for CC */
static char const ctrel01[] = {
	LEFT, S_FORCC,
	0
};

/*
 * ctrel02: reg vs indirect right
 * 68k: [right→INDR] cmp.w off(An),Dn
 * Z8002: [right→INDR nreg] cp Rn,off(Rm) (reversed, S_NEXT|S_INDR)
 */
static char const ctrel02z[] = {
	RIGHT, S_NEXT | S_INDR,
	OP, TRIGHT, ' ', LADDR, ',', ROFFSET, '(', NAR, ')', '\n',
	0
};

/*
 * ctrel03: reg vs easy right (compile right, compare)
 * 68k: [right→reg] EXRL; cmp.w Dn,Laddr
 * Z8002: [right→reg] EXRL; cp Laddr,Rn (reversed)
 */
static char const ctrel03[] = {
	RIGHT, 0,
	EXRL,
	OP, TLEFT, ' ', LADDR, ',', CR, '\n',
	0
};

/*
 * ct66p: long left vs unsigned int right (zero-extend right)
 * 68k: [left→reg] clr.l NR; move.w src,NR; cmp.l NR,CR
 * Z8002: [left→reg] clrl NR; ld NR,src; cpl CR,NR (reversed)
 */
static char const ct66pz[] = {
	LEFT, 0,
	CLRL, ' ', NR, '\n',
	MOV, TRIGHT, ' ', NR, ',', RADDR, '\n',
	OP, TLEFT, ' ', CR, ',', NR, '\n',
	0
};

/*
 * ctrel05: long left vs int right (via temp reg)
 * 68k: [left→reg] move.w src,An; cmp.l An,CR
 * Z8002: [left→reg] ld NR,src; cpl CR,NR
 */
static char const ctrel05z[] = {
	LEFT, 0,
	MOV, ' ', NR, ',', RADDR, '\n',
	OP, TLEFT, ' ', CR, ',', NR, '\n',
	0
};

/* ctshft00: just compile left (used when right is zero) */
static char const ctshft00[] = {
	LEFT, 0,
	0
};

/*
 * ct72p: float comparison via library call
 */
static char const ct72pz[] = {
	RIGHT, S_STACK,
	LEFT, S_STACK,
	JSR, ' ', OPCALL, '\n',
	POP8, 0
};

static struct skeleton const fc_rel[] = {
	{ SU_CONST | T_INT, SU_ZERO | T_ANY, ctshft00 },
	{ SU_CONST | T_INT, SU_CONST | T_INT, ctbop05 },
	/* Z8002: no SU_AREG entries */
	{ SU_ADDR | T_ANY, SU_ZERO | T_ANY, ctasg07 },
	{ SU_ADDR | T_LONG, SU_ZERO | T_ANY, ctasg07 },
	{ SU_ADDR | T_FLOAT, SU_ZERO | T_ANY, ctasg07 },
	{ SU_ANY | T_ANY | T_INDR, SU_ZERO | T_ANY, ctasg08 },
	{ SU_ANY | T_LONG | T_INDR, SU_ZERO | T_ANY, ctasg08 },
	{ SU_ANY | T_FLOAT | T_INDR, SU_ZERO | T_ANY, ctasg08 },
	{ SU_ANY | T_ANY, SU_ZERO | T_ANY, ctrel01 },
	{ SU_ANY | T_LONG, SU_ZERO | T_ANY, ctrel01 },
	{ SU_ANY | T_FLOAT, SU_ZERO | T_ANY, ctrel01 },
	{ SU_ADDR | T_ANY, SU_CONST | T_INT, ctasg09 },
	{ SU_ADDR | T_LONG, SU_CONST | T_INT, ctasg09 },
	{ SU_ANY | T_LONG | T_INDR, SU_CONST | T_INT, ctasg19z },
	{ SU_ANY | T_ANY | T_INDR, SU_CONST | T_INT, ctasg19z },
	{ SU_REG | T_CHAR, SU_ADDR | T_CHAR, ctasg09 },
	{ SU_REG | T_INT, SU_ADDR | T_INT, ctasg09 },
	{ SU_REG | T_LONG, SU_ADDR | T_LONG, ctasg09 },
	{ SU_REG | T_INT, SU_ANY | T_CHAR, ctasg18 },
	{ SU_REG | T_INT, SU_ANY | T_INT | T_INDR, ctrel02z },
	{ SU_REG | T_LONG, SU_ANY | T_LONG | T_INDR, ctrel02z },
	{ SU_REG | T_INT, SU_EASY | T_ANY, ctrel03 },
	{ SU_REG | T_LONG, SU_EASY | T_LONG, ctrel03 },
	{ SU_REG | T_LONG, SU_EASY | T_ANY, ctrel03 },
	{ SU_ANY | T_INT, SU_ADDR | T_INT, ctbop05 },
	{ SU_ANY | T_CHAR, SU_ADDR | T_CHAR, ctbop05 },
	{ SU_ANY | T_LONG, SU_ADDR | T_LONG, ctbop05 },
	{ SU_ANY | T_LONG, SU_CONST | T_INT, ctbop05 },
	{ SU_ANY | T_CHAR, SU_ADDR | T_INT, ctbop02 },
	{ SU_ANY | T_LONG, SU_ADDR | T_UNSN, ct66pz },
	{ SU_ANY | T_LONG, SU_ADDR | T_INT, ctrel05z },
	{ SU_ANY | T_ANY, SU_EASY | T_ANY, ctbop06 },
	{ SU_ANY | T_ANY, SU_EASY | T_LONG, ctbop06 },
	{ SU_ANY | T_LONG, SU_EASY | T_ANY, ctbop06 },
	{ SU_ANY | T_LONG, SU_EASY | T_LONG, ctbop06 },
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, ctbop07 },
	{ SU_ANY | T_ANY, SU_ANY | T_LONG, ctbop08 },
	{ SU_ANY | T_LONG, SU_ANY | T_LONG, ctbop08 },
	{ SU_ANY | T_ANY, SU_ANY | T_ANY, ctbop09 },
	{ SU_ANY | T_FLOAT, SU_ANY | T_FLOAT, ct72pz },
	{ 0, 0, NULL }
};


/* ================================================================
 * FUNCTION CALL skeleton group (fr_call=25)
 *
 * Z8002: "call addr" for direct, "call @Rn" for register indirect,
 *        "call offset(Rn)" for offset indirect.
 * JSR macro byte expands to "call" via strtab.
 * ================================================================ */

/* ctcal01: direct call — call _func */
static char const ctcal01[] = {
	JSR, ' ', LADDR, '\n',
	0
};

/*
 * ctcal02z: indirect call via pointer variable
 * 68k: [left→INDR] jsr off(An)
 * Z8002: S_NEXT|S_INDR for address; call off(Rn)
 */
static char const ctcal02z[] = {
	LEFT, S_NEXT | S_INDR,
	JSR, ' ', LOFFSET, '(', NAR, ')', '\n',
	0
};

/*
 * ctcal03z: register indirect call (function pointer in reg)
 * 68k: [left→reg] move.l Dn,An; jsr (An)
 * Z8002: [left→reg] call @CR (no move needed — unified regs)
 */
static char const ctcal03z[] = {
	LEFT, 0,
	JSR, ' ', '@', CR, '\n',
	0
};

static struct skeleton const fr_call[] = {
	{ SU_ADDR | T_ANY, SU_ANY | T_ANY, ctcal01 },
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, ctcal01 },
	{ SU_ANY | T_ANY | T_INDR, SU_ANY | T_ANY, ctcal02z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_ANY, ctcal02z },
	{ SU_ANY | T_ANY, SU_ANY | T_ANY, ctcal03z },
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, ctcal03z },
	{ 0, 0, NULL }
};


/* ================================================================
 * MULTIPLY skeleton group (fr_mult=13)
 *
 * Z8002 mult RRd,src: R(d+1) * src -> RRd
 * Multiplicand must be in R(d+1) = NR (odd register of pair).
 * Result: quotient in R(d+1), full 32-bit in RRd.
 * For int multiply, need low 16 bits: copy NR to CR after mult.
 * ================================================================ */

/* ctmul01z: int multiply by addressed right */
static char const ctmul01z[] = {
	LEFT, 0,
	MOV, ' ', NR, ',', CR, '\n',
	OP, ' ', CRPAIR, ',', RADDR, '\n',
	MOV, ' ', CR, ',', NR, '\n',
	0
};

/* ctmul02z: int multiply by indirect right */
static char const ctmul02z[] = {
	LEFT, 0,
	RIGHT, S_NEXT | S_INDR,
	MOV, ' ', NR, ',', CR, '\n',
	OP, ' ', CRPAIR, ',', ROFFSET, '(', NAR, ')', '\n',
	MOV, ' ', CR, ',', NR, '\n',
	0
};

/* ctmul03z: int multiply, right compiled to NR */
static char const ctmul03z[] = {
	LEFT, 0,
	RIGHT, S_NEXT,
	MOV, ' ', CR, ',', NR, '\n',
	OP, ' ', CRPAIR, ',', CR, '\n',
	MOV, ' ', CR, ',', NR, '\n',
	0
};

/* ctmul04z: int multiply via stack */
static char const ctmul04z[] = {
	RIGHT, S_STACK,
	LEFT, 0,
	MOV, ' ', NR, ',', CR, '\n',
	OP, ' ', CRPAIR, ',', POP, '\n',
	MOV, ' ', CR, ',', NR, '\n',
	0
};

/* ctmul05z: long multiply via library call */
static char const ctmul05z[] = {
	PSHL, RADDR, '\n',
	LEFT, S_STACK,
	JSR, ' ', OPCALL, '\n',
	POP8, 0
};

/* ctmul06z: long multiply — compile right, extend, push, compile left, call */
static char const ctmul06z[] = {
	RIGHT, 0,
	EXRL,
	PSHL, CR, '\n',
	LEFT, S_STACK,
	JSR, ' ', OPCALL, '\n',
	POP8, 0
};

/* ctmul07z: long multiply — compile left, extend, push, right to stack, call */
static char const ctmul07z[] = {
	LEFT, 0,
	EXLR,
	PSHL, CR, '\n',
	RIGHT, S_STACK,
	JSR, ' ', OPCALL, '\n',
	POP8, 0
};

static struct skeleton const fr_mult[] = {
	{ SU_ANY | T_CHAR, SU_ZERO | T_ANY, ctbop00 },
	{ SU_ANY | T_INT, SU_ZERO | T_ANY, ctbop01 },
	{ SU_ANY | T_LONG, SU_ZERO | T_ANY, ctbop01 },
	{ SU_ANY | T_ANY, SU_ADDR | T_INT, ctmul01z },
	{ SU_ANY | T_ANY, SU_EASY | T_INT | T_INDR, ctmul02z },
	{ SU_ANY | T_ANY, SU_EASY | T_ANY, ctmul03z },
	{ SU_ANY | T_ANY, SU_ANY | T_ANY, ctmul04z },
	{ SU_ANY | T_LONG, SU_CONST | T_INT, ctmul05z },
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, ctmul06z },
	{ SU_ANY | T_ANY, SU_ANY | T_LONG, ctmul07z },
	{ SU_ANY | T_LONG, SU_ANY | T_LONG, ctbop12 },
	{ SU_ANY | T_FLOAT, SU_ANY | T_FLOAT, ctbop12 },
	{ 0, 0, NULL }
};


/* ================================================================
 * DIVIDE skeleton group (fr_div=14)
 *
 * Z8002 div RRd,src: RRd / src -> quotient in R(d+1), remainder in R(d)
 * Dividend must be 32-bit in RRd (sign/zero-extended).
 * MODSWAP handles moving quotient to CR for DIV (not needed for MOD).
 * ================================================================ */

/* ctdiv00z: unsigned int / addressed right */
static char const ctdiv00z[] = {
	CLRL, ' ', CRPAIR, '\n',
	MOV, TLEFT, ' ', NR, ',', LADDR, '\n',
	OP, ' ', CRPAIR, ',', RADDR, '\n',
	MODSWAP, 0
};

/* ctdif01z: long left / addressed right (already 32-bit) */
static char const ctdif01z[] = {
	LEFT, 0,
	OP, ' ', CRPAIR, ',', RADDR, '\n',
	MODSWAP, 0
};

/* ctdiv02z: signed int / addressed right (sign-extend first) */
static char const ctdiv02z[] = {
	LEFT, 0,
	EXL,
	OP, ' ', CRPAIR, ',', RADDR, '\n',
	MODSWAP, 0
};

/* ctdiv03z: unsigned int / indirect right */
static char const ctdiv03z[] = {
	RIGHT, S_NEXT | S_INDR,
	CLRL, ' ', CRPAIR, '\n',
	MOV, TLEFT, ' ', NR, ',', LADDR, '\n',
	OP, ' ', CRPAIR, ',', ROFFSET, '(', NAR, ')', '\n',
	MODSWAP, 0
};

/* ctdiv4az: long left / indirect right */
static char const ctdiv4az[] = {
	LEFT, 0,
	RIGHT, S_NEXT | S_INDR,
	OP, ' ', CRPAIR, ',', ROFFSET, '(', NAR, ')', '\n',
	MODSWAP, 0
};

/* ctdiv04z: signed int / indirect right (sign-extend first) */
static char const ctdiv04z[] = {
	LEFT, 0,
	EXL,
	RIGHT, S_NEXT | S_INDR,
	OP, ' ', CRPAIR, ',', ROFFSET, '(', NAR, ')', '\n',
	MODSWAP, 0
};

/* ctdiv05z: unsigned int / easy right (compile right to NR) */
static char const ctdiv05z[] = {
	CLRL, ' ', CRPAIR, '\n',
	MOV, ' ', NR, ',', LADDR, '\n',
	RIGHT, S_NEXT,
	OP, ' ', CRPAIR, ',', NR, '\n',
	MODSWAP, 0
};

/* ctdiv06z: signed int / easy right (sign-extend, compile right) */
static char const ctdiv06z[] = {
	LEFT, 0,
	EXL,
	RIGHT, S_NEXT,
	OP, ' ', CRPAIR, ',', NR, '\n',
	MODSWAP, 0
};

/* ctdiv07z: long left / easy right (already 32-bit) */
static char const ctdiv07z[] = {
	LEFT, 0,
	RIGHT, S_NEXT,
	OP, ' ', CRPAIR, ',', NR, '\n',
	MODSWAP, 0
};

/* ctdiv08z: long / via stack */
static char const ctdiv08z[] = {
	RIGHT, S_STACK,
	LEFT, 0,
	OP, ' ', CRPAIR, ',', POP, '\n',
	MODSWAP, 0
};

/* ctdiv09z: signed int / via stack */
static char const ctdiv09z[] = {
	RIGHT, S_STACK,
	LEFT, 0,
	EXL,
	OP, ' ', CRPAIR, ',', POP, '\n',
	MODSWAP, 0
};

/* ctdiv10z: long / via stack (no EXL needed) */
static char const ctdiv10z[] = {
	RIGHT, S_STACK,
	LEFT, 0,
	OP, ' ', CRPAIR, ',', POP, '\n',
	MODSWAP, 0
};

static struct skeleton const fr_div[] = {
	{ SU_ADDR | T_UNSN, SU_ADDR | T_INT, ctdiv00z },
	{ SU_ANY | T_LONG, SU_ADDR | T_INT, ctdif01z },
	{ SU_ANY | T_ANY, SU_ADDR | T_INT, ctdiv02z },
	{ SU_ADDR | T_UNSN, SU_EASY | T_INT | T_INDR, ctdiv03z },
	{ SU_ANY | T_LONG, SU_EASY | T_INT | T_INDR, ctdiv4az },
	{ SU_ANY | T_ANY, SU_EASY | T_INT | T_INDR, ctdiv04z },
	{ SU_ADDR | T_UNSN, SU_EASY | T_ANY, ctdiv05z },
	{ SU_ANY | T_ANY, SU_EASY | T_ANY, ctdiv06z },
	{ SU_ANY | T_LONG, SU_EASY | T_ANY, ctdiv07z },
	{ SU_ANY | T_LONG, SU_CONST | T_INT, ctdiv08z },
	{ SU_ANY | T_ANY, SU_ANY | T_ANY, ctdiv09z },
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, ctdiv10z },
	{ SU_ANY | T_ANY, SU_ANY | T_LONG, ctmul07z },
	{ SU_ANY | T_LONG, SU_ANY | T_LONG, ctbop12 },
	{ SU_ANY | T_FLOAT, SU_ANY | T_FLOAT, ctbop12 },
	{ 0, 0, NULL }
};


/* ================================================================
 * LONG DIVIDE skeleton group (fr_ldiv=42)
 * ================================================================ */

/* ctldiv1z: long divide — compile right, extend, push, left to stack, call */
static char const ctldiv1z[] = {
	RIGHT, 0,
	EXRL,
	PSHL, CR, '\n',
	LEFT, S_STACK,
	JSR, ' ', OPCALL, '\n',
	POP8, 0
};

/* ctldiv2z: long divide — indirect right, extend, push, left to stack, call */
static char const ctldiv2z[] = {
	RIGHT, S_NEXT | S_INDR,
	MOV, ' ', ROFFSET, '(', NAR, ')', ',', CR, '\n',
	EXRL,
	PSHL, CR, '\n',
	LEFT, S_STACK,
	JSR, ' ', OPCALL, '\n',
	POP8, 0
};

/* ctldiv3z: long divide — compile right to NR, push, left to stack, call */
static char const ctldiv3z[] = {
	RIGHT, S_NEXT,
	EXRLN,
	PSHL, NR, '\n',
	LEFT, S_STACK,
	JSR, ' ', OPCALL, '\n',
	POP8, 0
};

static struct skeleton const fr_ldiv[] = {
	{ SU_ADDR | T_UNSN, SU_ADDR | T_INT, ctdiv00z },
	{ SU_ANY | T_ANY, SU_ADDR | T_INT, ctdiv02z },
	{ SU_ANY | T_LONG, SU_ADDR | T_INT, ctldiv1z },
	{ SU_ADDR | T_UNSN, SU_EASY | T_INT | T_INDR, ctdiv03z },
	{ SU_ANY | T_ANY, SU_EASY | T_INT | T_INDR, ctdiv04z },
	{ SU_ANY | T_LONG, SU_EASY | T_INT | T_INDR, ctldiv2z },
	{ SU_ADDR | T_UNSN, SU_EASY | T_ANY, ctdiv05z },
	{ SU_ANY | T_ANY, SU_EASY | T_ANY, ctdiv06z },
	{ SU_ANY | T_LONG, SU_EASY | T_ANY, ctldiv3z },
	{ SU_ANY | T_LONG, SU_CONST | T_INT, ctmul05z },
	{ SU_ANY | T_ANY, SU_ANY | T_ANY, ctdiv09z },
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, ctmul06z },
	{ SU_ANY | T_ANY, SU_ANY | T_LONG, ctmul07z },
	{ SU_ANY | T_LONG, SU_ANY | T_LONG, ctbop12 },
	{ SU_ANY | T_FLOAT, SU_ANY | T_FLOAT, ctbop12 },
	{ 0, 0, NULL }
};


/* ================================================================
 * SHIFT skeleton group (fr_shft=15)
 *
 * Z8002: sla Rn,#count (shift left arithmetic) — dst first, then count.
 * Same as 68k operand order for shifts: asl #count,Dn → sla Rn,#count
 * Wait — 68k shift is: asl #count,Dn (count,dst)
 *            Z8002 is: sla Rn,#count (dst,count) — REVERSED!
 * So shift skeletons need operand reversal.
 * ================================================================ */

/* ctshf01z: shift by small constant (reversed from 68k) */
static char const ctshf01z[] = {
	LEFT, 0,
	OP, TLEFTL, ' ', CR, ',', RADDR, '\n',
	0
};

/* ctshf02z: shift by variable amount */
static char const ctshf02z[] = {
	LEFT, 0,
	RIGHT, S_NEXT,
	OP, TLEFTL, ' ', CR, ',', NR, '\n',
	0
};

static struct skeleton const fr_shft[] = {
	{ SU_ANY | T_ANY, SU_ZERO | T_ANY, ctshft00 },
	{ SU_ANY | T_LONG, SU_ZERO | T_ANY, ctshft00 },
	{ SU_ANY | T_ANY, SU_SMALL | T_INT, ctshf01z },
	{ SU_ANY | T_LONG, SU_SMALL | T_INT, ctshf01z },
	{ SU_ANY | T_ANY, SU_ANY | T_ANY, ctshf02z },
	{ SU_ANY | T_ANY, SU_ANY | T_LONG, ctshf02z },
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, ctshf02z },
	{ SU_ANY | T_LONG, SU_ANY | T_LONG, ctshf02z },
	{ 0, 0, NULL }
};


/* ================================================================
 * XOR skeleton group (fr_xor=16)
 *
 * Z8002 xor: xor Rn,src (dst first — reversed from 68k).
 * Easy cases use ctbop06 (shared with fr_add).
 * Hard cases need stack-based approach.
 * ================================================================ */

/* ctxor01z: xor via stack — push left, compile right, xor with POP */
static char const ctxor01z[] = {
	LEFT, S_STACK,
	RIGHT, 0,
	OP, ' ', CR, ',', POP, '\n',
	0
};

/* ct71rz: long xor via stack */
static char const ct71rz[] = {
	LEFT, S_STACK,
	RIGHT, 0,
	OP, TLEFT, ' ', CR, ',', POP, '\n',
	0
};

static struct skeleton const fr_xor[] = {
	{ SU_ANY | T_ANY, SU_EASY | T_ANY, ctbop06 },
	{ SU_ANY | T_ANY, SU_EASY | T_LONG, ctbop06 },
	{ SU_ANY | T_LONG, SU_EASY | T_ANY, ctbop06 },
	{ SU_ANY | T_LONG, SU_EASY | T_LONG, ctbop06 },
	{ SU_ANY | T_ANY, SU_ANY | T_ANY, ctxor01z },
	{ SU_ANY | T_LONG, SU_ANY | T_LONG, ct71rz },
	{ 0, 0, NULL }
};


/* ================================================================
 * NEGATE/COMPLEMENT skeleton group (fr_neg=17)
 *
 * Z8002: neg/com are unary — single operand, no reversal needed.
 * OP ' ' CR → "neg R0" or "com R0"
 * ================================================================ */

/* ctneg01: negate/complement char */
static char const ctneg01[] = {
	LEFT, 0,
	OP, ' ', CR, '\n',
	0
};

/* ctneg02: negate/complement int/long */
static char const ctneg02[] = {
	LEFT, 0,
	OP, TLEFT, ' ', CR, '\n',
	0
};

/* ctneg03: float negate via library call */
static char const ctneg03[] = {
	LEFT, S_STACK,
	JSR, ' ', OPCALL, '\n',
	POP4,
	0
};

static struct skeleton const fr_neg[] = {
	{ SU_ANY | T_CHAR, SU_ANY | T_ANY, ctneg01 },
	{ SU_ANY | T_INT, SU_ANY | T_ANY, ctneg02 },
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, ctneg02 },
	{ SU_ANY | T_FLOAT, SU_ANY | T_ANY, ctneg03 },
	{ 0, 0, NULL }
};


/* ================================================================
 * POST INCREMENT/DECREMENT skeleton group (fr_postop=19)
 *
 * Post-op: return old value, then modify.
 * Z8002 operand reversal applies to the inc/dec operations.
 * ================================================================ */

/* ctpst01z: addressed variable post-op (load old, then modify in place) */
static char const ctpst01z[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	AOP, TLEFT, ' ', LADDR, '+', '\n',
	0
};

/* ctpst02z: addressed variable post-op (float, via library) */
static char const ctpst02z[] = {
	MOVL, ' ', CR, ',', LADDR, '-', '\n',
	PSH, LADDR, '+', '\n',
	JSR, ' ', OPCALL, '\n',
	POP4, 0
};

/* ctpst03z: indirect float post-op */
static char const ctpst03z[] = {
	LEFT, S_NEXT | S_INDR,
	MOVL, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	PSH, LOFFSET, '(', NAR, ')', '\n',
	JSR, ' ', OPCALL, '\n',
	POP4,
	MOVL, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ctpst04z: indirect variable post-op */
static char const ctpst04z[] = {
	LEFT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	AOP, TLEFT, ' ', LOFFSET, '(', NAR, ')', '\n',
	0
};

static struct skeleton const fr_postop[] = {
	{ SU_ANY | T_FLOAT | T_INDR, SU_ANY | T_ANY, ctpst03z },
	{ SU_ANY | T_FLOAT, SU_ANY | T_ANY, ctpst02z },
	{ SU_ADDR | T_ANY, SU_ANY | T_ANY, ctpst01z },
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, ctpst01z },
	{ SU_ANY | T_ANY | T_INDR, SU_ANY | T_ANY, ctpst04z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_ANY, ctpst04z },
	{ 0, 0, NULL }
};


/* ================================================================
 * COMPOUND ASSIGN skeleton group (fr_eqop=18, fe_eqop=1)
 *
 * +=, -=, &=, |= patterns.
 * Z8002 operand reversal: OP src,dst → OP dst,src
 * S_INDR+CAR → S_NEXT|S_INDR+NAR
 * ================================================================ */

/* ctreo01z: addressed var op zero/const (unary or direct) */
static char const ctreo01z[] = {
	AOP, TLEFT, ' ', LADDR, '\n',
	MOV, TLEFT, ' ', CR, ',', LADDR, '\n',
	0
};

/* ctreo02z: unsigned addr op long right */
static char const ctreo02z[] = {
	CLRL, ' ', CR, '\n',
	MOV, TLEFT, ' ', NR, ',', LADDR, '\n',
	OP, TLEFT, ' ', CR, ',', RADDR, '\n',
	MOV, TLEFT, ' ', LADDR, ',', NR, '\n',
	0
};

/* ctreo03z: addressed var op addressed right */
static char const ctreo03z[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	OP, TRIGHT, ' ', CR, ',', RADDR, '\n',
	MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctreo24z: char var op addr right (extend char, operate, truncate back) */
static char const ctreo24z[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	EXTW, ' ', CR, '\n',
	OP, TRIGHT, ' ', CR, ',', RADDR, '\n',
	MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctreo04z: unsigned addr op long indirect right */
static char const ctreo04z[] = {
	RIGHT, S_NEXT | S_INDR,
	CLRL, ' ', CR, '\n',
	MOV, TLEFT, ' ', NR, ',', LADDR, '\n',
	OP, TRIGHT, ' ', CR, ',', ROFFSET, '(', NAR, ')', '\n',
	MOV, TLEFT, ' ', LADDR, ',', NR, '\n',
	0
};

/* ctreo05z: addressed var op indirect right */
static char const ctreo05z[] = {
	RIGHT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	OP, TRIGHT, ' ', CR, ',', ROFFSET, '(', NAR, ')', '\n',
	MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctreo26z: char var op indirect right (extend, operate) */
static char const ctreo26z[] = {
	RIGHT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	EXTW, ' ', CR, '\n',
	OP, TRIGHT, ' ', CR, ',', ROFFSET, '(', NAR, ')', '\n',
	MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctreo06z: float compound assign via library */
static char const ctreo06z[] = {
	PSH, RADDR, '\n',
	PSH, LADDR, '-', '\n',
	JSR, ' ', OPCALL, '\n',
	POP8,
	MOVL, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctreo07z: unsigned addr op easy long right (zero-extend, operate) */
static char const ctreo07z[] = {
	CLRL, ' ', CR, '\n',
	MOV, TLEFT, ' ', NR, ',', LADDR, '\n',
	RIGHT, S_NEXT,
	EXRLN,
	OP, TLEFT, ' ', CR, ',', NR, '\n',
	MOV, TLEFT, ' ', LADDR, ',', NR, '\n',
	0
};

/* ctreo08z: addressed var op easy right */
static char const ctreo08z[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	RIGHT, S_NEXT,
	EXRLN,
	OP, TEITHER, ' ', CR, ',', NR, '\n',
	MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctreo27z: char var op easy right (extend, operate) */
static char const ctreo27z[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	EXTW, ' ', CR, '\n',
	RIGHT, S_NEXT,
	EXRLN,
	OP, TEITHER, ' ', CR, ',', NR, '\n',
	MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctreo09z: unsigned addr op long any right (via stack) */
static char const ctreo09z[] = {
	RIGHT, S_STACK,
	CLRL, ' ', CR, '\n',
	MOV, TLEFT, ' ', NR, ',', LADDR, '\n',
	OP, TLEFT, ' ', CR, ',', POP, '\n',
	MOV, TLEFT, ' ', LADDR, ',', NR, '\n',
	0
};

/* ctreo10z: addressed var op any right (via stack) */
static char const ctreo10z[] = {
	RIGHT, S_STACK,
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	OP, TRIGHT, ' ', CR, ',', POP, '\n',
	MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctreo28z: char var op any right (extend, operate via stack) */
static char const ctreo28z[] = {
	RIGHT, S_STACK,
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	EXTW, ' ', CR, '\n',
	OP, TRIGHT, ' ', CR, ',', POP, '\n',
	MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctreo11z: addressed var op char right (compile right to reg) */
static char const ctreo11z[] = {
	RIGHT, 0,
	EXRL,
	MOV, TLEFT, ' ', NR, ',', LADDR, '-', '\n',
	OP, TLEFT, ' ', NR, ',', CR, '\n',
	MOV, TLEFT, ' ', LADDR, '+', ',', NR, '\n',
	MOV, TLEFT, ' ', CR, ',', NR, '\n',
	0
};

/* ctreo12z: char var op char right */
static char const ctreo12z[] = {
	RIGHT, 0,
	EXRL,
	MOV, TLEFT, ' ', NR, ',', LADDR, '-', '\n',
	OP, ' ', NR, ',', CR, '\n',
	MOV, TLEFT, ' ', LADDR, '+', ',', NR, '\n',
	MOV, ' ', CR, ',', NR, '\n',
	0
};

/* ctreo13z: indirect char var op addr right */
static char const ctreo13z[] = {
	LEFT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	EXTW, ' ', CR, '\n',
	OP, TRIGHT, ' ', CR, ',', RADDR, '\n',
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ctreo14z: indirect var op zero/const */
static char const ctreo14z[] = {
	LEFT, S_NEXT | S_INDR,
	AOP, TLEFT, ' ', LOFFSET, '(', NAR, ')', '\n',
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	0
};

/* ctreo15z: indirect var op addr right */
static char const ctreo15z[] = {
	LEFT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	OP, TRIGHT, ' ', CR, ',', RADDR, '\n',
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ctreo16z: indirect var op easy right */
static char const ctreo16z[] = {
	LEFT, S_NEXT | S_INDR,
	RIGHT, S_NEXT,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	OP, TEITHER, ' ', CR, ',', NR, '\n',
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ct35oz: indirect unsigned char op easy int (zero-extend) */
static char const ct35oz[] = {
	LEFT, S_NEXT | S_INDR,
	CLR, ' ', CR, '\n',
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	RIGHT, S_NEXT,
	OP, TEITHER, ' ', CR, ',', NR, '\n',
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ctreo17z: indirect signed char op easy int (sign-extend) */
static char const ctreo17z[] = {
	LEFT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	EXTW, ' ', CR, '\n',
	RIGHT, S_NEXT,
	OP, TEITHER, ' ', CR, ',', NR, '\n',
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ct37oz: indirect unsigned char op easy char */
static char const ct37oz[] = {
	LEFT, S_NEXT | S_INDR,
	CLR, ' ', CR, '\n',
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	RIGHT, S_NEXT,
	OP, ' ', CR, ',', NR, '\n',
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ctreo18z: indirect signed char op easy char (sign-extend) */
static char const ctreo18z[] = {
	LEFT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	EXTW, ' ', CR, '\n',
	RIGHT, S_NEXT,
	OP, ' ', CR, ',', NR, '\n',
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ctreo19z: indirect float op via library */
static char const ctreo19z[] = {
	LEFT, S_NEXT | S_INDR,
	PSH, RADDR, '\n',
	PSH, LOFFSET, '(', NAR, ')', '\n',
	JSR, ' ', OPCALL, '\n',
	POP8,
	MOVL, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ctreo20z: indirect int op any int (via stack) */
static char const ctreo20z[] = {
	RIGHT, S_STACK,
	LEFT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	OP, TRIGHT, ' ', CR, ',', POP, '\n',
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ct41oz: indirect unsigned char op any int (via stack, zero-extend) */
static char const ct41oz[] = {
	RIGHT, S_STACK,
	LEFT, S_NEXT | S_INDR,
	CLR, ' ', CR, '\n',
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	OP, TRIGHT, ' ', CR, ',', POP, '\n',
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ctreo21z: indirect signed char op any int (via stack, sign-extend) */
static char const ctreo21z[] = {
	RIGHT, S_STACK,
	LEFT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	EXTW, ' ', CR, '\n',
	OP, TRIGHT, ' ', CR, ',', POP, '\n',
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ctreo22z: indirect long op any (via stack) */
static char const ctreo22z[] = {
	RIGHT, S_STACK,
	LEFT, S_NEXT | S_INDR,
	MOVL, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	OP, TLEFT, ' ', CR, ',', POP, '\n',
	MOVL, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ctreo23z: indirect char op any char (via stack) */
static char const ctreo23z[] = {
	RIGHT, S_STACK,
	LEFT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	OP, ' ', CR, ',', POP, '\n',
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

static struct skeleton const fr_eqop[] = {
	{ SU_ANY | T_FLOAT | T_INDR, SU_ONE | T_ANY, ctpst03z },
	{ SU_ANY | T_FLOAT, SU_ONE | T_ANY, ctpst02z },
	{ SU_ADDR | T_ANY, SU_ZERO | T_ANY, ctreo01z },
	{ SU_ANY | T_LONG, SU_ZERO | T_ANY, ctreo01z },
	{ SU_ADDR | T_INT, SU_CONST | T_INT, ctreo01z },
	{ SU_ADDR | T_LONG, SU_CONST | T_INT, ctreo01z },
	{ SU_ADDR | T_UNSN, SU_ADDR | T_LONG, ctreo02z },
	{ SU_ADDR | T_CHAR, SU_CONST | T_INT, ctreo24z },
	{ SU_ADDR | T_CHAR, SU_ADDR | T_INT, ctreo24z },
	{ SU_ADDR | T_CHAR, SU_ADDR | T_LONG, ctreo24z },
	{ SU_ADDR | T_CHAR, SU_CONST | T_INT, ctreo03z },
	{ SU_ADDR | T_INT, SU_ADDR | T_INT, ctreo03z },
	{ SU_ADDR | T_INT, SU_ADDR | T_LONG, ctreo03z },
	{ SU_ADDR | T_LONG, SU_ADDR | T_LONG, ctreo03z },
	{ SU_ADDR | T_UNSN, SU_ANY | T_LONG | T_INDR, ctreo04z },
	{ SU_ADDR | T_CHAR, SU_ANY | T_INT | T_INDR, ctreo26z },
	{ SU_ADDR | T_CHAR, SU_ANY | T_LONG | T_INDR, ctreo26z },
	{ SU_ADDR | T_INT, SU_ANY | T_INT | T_INDR, ctreo05z },
	{ SU_ADDR | T_INT, SU_ANY | T_LONG | T_INDR, ctreo05z },
	{ SU_ADDR | T_LONG, SU_ANY | T_LONG | T_INDR, ctreo05z },
	{ SU_ADDR | T_FLOAT, SU_ANY | T_FLOAT, ctreo06z },
	{ SU_ADDR | T_UNSN, SU_EASY | T_LONG, ctreo07z },
	{ SU_ADDR | T_CHAR, SU_EASY | T_ANY, ctreo27z },
	{ SU_ADDR | T_CHAR, SU_EASY | T_LONG, ctreo27z },
	{ SU_ADDR | T_INT, SU_EASY | T_ANY, ctreo08z },
	{ SU_ADDR | T_INT, SU_EASY | T_LONG, ctreo08z },
	{ SU_ADDR | T_LONG, SU_EASY | T_ANY, ctreo08z },
	{ SU_ADDR | T_LONG, SU_EASY | T_LONG, ctreo08z },
	{ SU_ADDR | T_UNSN, SU_ANY | T_LONG, ctreo09z },
	{ SU_ADDR | T_CHAR, SU_ANY | T_INT, ctreo28z },
	{ SU_ADDR | T_CHAR, SU_ANY | T_LONG, ctreo28z },
	{ SU_ADDR | T_INT, SU_ANY | T_INT, ctreo10z },
	{ SU_ADDR | T_INT, SU_ANY | T_LONG, ctreo10z },
	{ SU_ADDR | T_LONG, SU_ANY | T_LONG, ctreo10z },
	{ SU_ADDR | T_INT, SU_ANY | T_CHAR, ctreo11z },
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, ctreo11z },
	{ SU_ADDR | T_CHAR, SU_ANY | T_CHAR, ctreo12z },
	{ SU_ANY | T_CHAR | T_INDR, SU_ADDR | T_INT, ctreo13z },
	{ SU_ANY | T_CHAR | T_INDR, SU_ADDR | T_LONG, ctreo13z },
	{ SU_ANY | T_INT | T_INDR, SU_ZERO | T_ANY, ctreo14z },
	{ SU_ANY | T_LONG | T_INDR, SU_ZERO | T_ANY, ctreo14z },
	{ SU_ANY | T_INT | T_INDR, SU_CONST | T_INT, ctreo14z },
	{ SU_ANY | T_LONG | T_INDR, SU_CONST | T_INT, ctreo14z },
	{ SU_ANY | T_INT | T_INDR, SU_ADDR | T_INT, ctreo15z },
	{ SU_ANY | T_INT | T_INDR, SU_ADDR | T_LONG, ctreo15z },
	{ SU_ANY | T_LONG | T_INDR, SU_ADDR | T_LONG, ctreo15z },
	{ SU_ANY | T_INT | T_INDR, SU_EASY | T_ANY, ctreo16z },
	{ SU_ANY | T_INT | T_INDR, SU_EASY | T_LONG, ctreo16z },
	{ SU_ANY | T_LONG | T_INDR, SU_EASY | T_ANY, ctreo16z },
	{ SU_ANY | T_LONG | T_INDR, SU_EASY | T_LONG, ctreo16z },
	{ SU_ANY | T_UCHAR | T_INDR, SU_EASY | T_INT, ct35oz },
	{ SU_ANY | T_UCHAR | T_INDR, SU_EASY | T_LONG, ct35oz },
	{ SU_ANY | T_CHAR | T_INDR, SU_EASY | T_INT, ctreo17z },
	{ SU_ANY | T_CHAR | T_INDR, SU_EASY | T_LONG, ctreo17z },
	{ SU_ANY | T_UCHAR | T_INDR, SU_EASY | T_CHAR, ct37oz },
	{ SU_ANY | T_CHAR | T_INDR, SU_EASY | T_CHAR, ctreo18z },
	{ SU_ANY | T_FLOAT | T_INDR, SU_ANY | T_FLOAT, ctreo19z },
	{ SU_ANY | T_INT | T_INDR, SU_ANY | T_INT, ctreo20z },
	{ SU_ANY | T_UCHAR | T_INDR, SU_ANY | T_INT, ct41oz },
	{ SU_ANY | T_UCHAR | T_INDR, SU_ANY | T_LONG, ct41oz },
	{ SU_ANY | T_CHAR | T_INDR, SU_ANY | T_INT, ctreo21z },
	{ SU_ANY | T_CHAR | T_INDR, SU_ANY | T_LONG, ctreo21z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_ANY, ctreo22z },
	{ SU_ANY | T_CHAR | T_INDR, SU_ANY | T_CHAR, ctreo23z },
	{ 0, 0, NULL }
};


/* --- fe_eqop (1): compound assign for effect --- */

/* cteop01z: addr var op indirect right */
static char const cteop01z[] = {
	RIGHT, S_NEXT | S_INDR,
	OP, TLEFT, ' ', LADDR, ',', ROFFSET, '(', NAR, ')', '\n',
	0
};

/* cteop02z: indirect var op indirect right (both easy) */
static char const cteop02z[] = {
	LEFT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	RIGHT, S_NEXT | S_INDR,
	OP, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* cteop03z: addr var op indirect right (via reg) */
static char const cteop03z[] = {
	RIGHT, S_NEXT | S_INDR,
	MOV, TRIGHT, ' ', CR, ',', ROFFSET, '(', NAR, ')', '\n',
	EXRL,
	OP, TLEFT, ' ', LADDR, ',', CR, '\n',
	0
};

/* cteop04z: addr var op any right */
static char const cteop04z[] = {
	RIGHT, 0,
	EXRL,
	OP, TLEFT, ' ', LADDR, ',', CR, '\n',
	0
};

/* cteop05z: indirect var op any right (via stack) */
static char const cteop05z[] = {
	RIGHT, 0,
	EXRL,
	LEFT, S_NEXT | S_INDR,
	OP, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* cteop06z: long reg op unsigned addr right (zero-extend, operate) */
static char const cteop06z[] = {
	CLRL, ' ', NR, '\n',
	MOV, TRIGHT, ' ', NR, ',', RADDR, '\n',
	OP, TLEFT, ' ', CR, ',', NR, '\n',
	0
};

/* cteop07z: long reg op addr right (via temp reg) */
static char const cteop07z[] = {
	MOV, ' ', NR, ',', RADDR, '\n',
	OP, TLEFT, ' ', CR, ',', NR, '\n',
	0
};

/* cteop08z: long indirect op long right (via stack) */
static char const cteop08z[] = {
	RIGHT, S_STACK,
	LEFT, S_NEXT | S_INDR,
	MOVL, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	OP, TLEFT, ' ', CR, ',', POP, '\n',
	MOVL, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* cteop09z: long indirect op addr right */
static char const cteop09z[] = {
	LEFT, S_NEXT | S_INDR,
	MOVL, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	OP, TLEFT, ' ', CR, ',', RADDR, '\n',
	MOVL, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* cteop10z: long indirect op any right (via stack) */
static char const cteop10z[] = {
	RIGHT, 0,
	EXRL,
	LEFT, S_NEXT | S_INDR,
	MOVL, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	OP, TLEFT, ' ', CR, ',', NR, '\n',
	MOVL, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

static struct skeleton const fe_eqop[] = {
	{ SU_ADDR | T_ANY, SU_ZERO | T_ANY, ctasg09 },
	{ SU_ANY | T_LONG, SU_ZERO | T_ANY, ctasg09 },
	{ SU_ADDR | T_ANY, SU_CONST | T_INT, ctasg09 },
	{ SU_ADDR | T_LONG, SU_CONST | T_INT, ctasg09 },
	{ SU_ANY | T_ANY | T_INDR, SU_ZERO | T_ANY, ctasg19z },
	{ SU_ANY | T_LONG | T_INDR, SU_ZERO | T_ANY, ctasg19z },
	{ SU_ANY | T_ANY | T_INDR, SU_CONST | T_INT, ctasg19z },
	{ SU_ANY | T_LONG | T_INDR, SU_CONST | T_INT, ctasg19z },
	/* Z8002: no SU_AREG entries */
	{ SU_ADDR | T_INT, SU_REG | T_INT, ctasg09 },
	{ SU_REG | T_INT, SU_ADDR | T_INT, ctasg09 },
	{ SU_ADDR | T_LONG, SU_REG | T_LONG, ctasg09 },
	{ SU_REG | T_LONG, SU_ADDR | T_LONG, ctasg09 },
	{ SU_ADDR | T_INT, SU_ADDR | T_INT, ctasg02 },
	{ SU_ADDR | T_CHAR, SU_ADDR | T_CHAR, ctasg02 },
	{ SU_ADDR | T_LONG, SU_ADDR | T_LONG, ctasg02 },
	{ SU_ADDR | T_INT, SU_ANY | T_INT | T_INDR, cteop01z },
	{ SU_ADDR | T_INT, SU_ANY | T_ANY, ctasg02 },
	{ SU_EASY | T_INT | T_INDR, SU_ANY | T_INT | T_INDR, cteop02z },
	{ SU_EASY | T_LONG | T_INDR, SU_ANY | T_LONG | T_INDR, cteop02z },
	{ SU_ADDR | T_ANY, SU_ANY | T_INT | T_INDR, cteop03z },
	{ SU_ADDR | T_ANY, SU_ANY | T_ANY, cteop04z },
	{ SU_EASY | T_INT | T_INDR, SU_EASY | T_ANY, ctasg05z },
	{ SU_ANY | T_INT | T_INDR, SU_ANY | T_ANY, ctasg06z },
	{ SU_ANY | T_ANY | T_INDR, SU_ANY | T_ANY, cteop05z },
	{ SU_REG | T_LONG, SU_ADDR | T_UNSN, cteop06z },
	{ SU_REG | T_LONG, SU_ADDR | T_ANY, cteop07z },
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, ctasg02 },
	{ SU_ADDR | T_LONG, SU_ANY | T_LONG | T_INDR, cteop01z },
	{ SU_ADDR | T_LONG, SU_ANY | T_LONG, ctasg02 },
	{ SU_EASY | T_LONG | T_INDR, SU_EASY | T_LONG, ctasg05z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_LONG, cteop08z },
	{ SU_ANY | T_LONG | T_INDR, SU_ADDR | T_ANY, cteop09z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_ANY, cteop10z },
	{ 0, 0, NULL }
};


/* ================================================================
 * COMPOUND MULTIPLY skeleton group (fr_eqmult=21, fe_eqmult=38)
 * ================================================================ */

static struct skeleton const fr_eqmult[] = {
	{ SU_ADDR | T_CHAR, SU_ADDR | T_INT, ctreo24z },
	{ SU_ADDR | T_INT, SU_ADDR | T_INT, ctreo03z },
	{ SU_ADDR | T_CHAR, SU_ANY | T_INT | T_INDR, ctreo26z },
	{ SU_ADDR | T_INT, SU_ANY | T_INT | T_INDR, ctreo05z },
	{ SU_ADDR | T_FLOAT, SU_ANY | T_FLOAT, ctreo06z },
	{ SU_ADDR | T_CHAR, SU_EASY | T_ANY, ctreo27z },
	{ SU_ADDR | T_INT, SU_EASY | T_ANY, ctreo08z },
	{ SU_ADDR | T_CHAR, SU_ANY | T_INT, ctreo28z },
	{ SU_ADDR | T_INT, SU_ANY | T_INT, ctreo10z },
	{ SU_ADDR | T_INT, SU_ANY | T_CHAR, ctreo11z },
	{ SU_ADDR | T_CHAR, SU_ANY | T_CHAR, ctreo12z },
	{ SU_ANY | T_CHAR | T_INDR, SU_ADDR | T_INT, ctreo13z },
	{ SU_ANY | T_INT | T_INDR, SU_ADDR | T_INT, ctreo15z },
	{ SU_ANY | T_INT | T_INDR, SU_EASY | T_ANY, ctreo16z },
	{ SU_ANY | T_CHAR | T_INDR, SU_EASY | T_INT, ctreo17z },
	{ SU_ANY | T_CHAR | T_INDR, SU_EASY | T_CHAR, ctreo18z },
	{ SU_ANY | T_FLOAT | T_INDR, SU_ANY | T_FLOAT, ctreo19z },
	{ SU_ANY | T_INT | T_INDR, SU_ANY | T_INT, ctreo20z },
	{ SU_ANY | T_CHAR | T_INDR, SU_ANY | T_INT, ctreo21z },
	{ SU_ANY | T_CHAR | T_INDR, SU_ANY | T_CHAR, ctreo23z },
	{ 0, 0, NULL }
};

/* ct89gz: unsigned multiply for effect */
static char const ct89gz[] = {
	AOP, ' ', CR, ',', LADDR, '\n',
	0
};

static struct skeleton const fe_eqmult[] = {
	{ SU_REG | T_INT, SU_ADDR | T_INT, ctasg09 },
	{ SU_REG | T_UNSN, SU_ADDR | T_INT, ct89gz },
	{ SU_REG | T_UNSN, SU_ADDR | T_UNSN, ct89gz },
	{ 0, 0, NULL }
};


/* ================================================================
 * COMPOUND DIVIDE skeleton group (fr_eqdiv=22, fe_eqdiv=39, fe_eqmod=40)
 *
 * Z8002: div RRd,src puts quotient in R(d+1), remainder in R(d).
 * For compound /=: load var, extend to long, divide, store quotient back.
 * For compound %=: same but store remainder.
 * MODSWAP handles the quotient/remainder selection.
 * ================================================================ */

/* ctedv01z: unsigned compound divide */
static char const ctedv01z[] = {
	CLRL, ' ', CRPAIR, '\n',
	MOV, TLEFT, ' ', NR, ',', LADDR, '-', '\n',
	OP, ' ', CRPAIR, ',', RADDR, '\n',
	MODSWAP, MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctedv12z: signed char compound divide (extend byte to word, then to long) */
static char const ctedv12z[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	EXTW, ' ', CR, '\n',
	EXL,
	OP, ' ', CRPAIR, ',', RADDR, '\n',
	MODSWAP, MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctedv02z: signed int compound divide */
static char const ctedv02z[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	EXL,
	OP, ' ', CRPAIR, ',', RADDR, '\n',
	MODSWAP, MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctedv03z: unsigned compound divide by easy right */
static char const ctedv03z[] = {
	CLRL, ' ', CRPAIR, '\n',
	MOV, TLEFT, ' ', NR, ',', LADDR, '-', '\n',
	RIGHT, S_NEXT,
	OP, ' ', CRPAIR, ',', NR, '\n',
	MODSWAP, MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctedv04z: long compound divide by addressed right */
static char const ctedv04z[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	OP, ' ', CRPAIR, ',', RADDR, '\n',
	MODSWAP, EXRL,
	MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ct01vz: signed char compound divide via stack */
static char const ct01vz[] = {
	RIGHT, S_STACK,
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	EXTW, ' ', CR, '\n',
	EXL,
	OP, ' ', CRPAIR, ',', POP, '\n',
	MODSWAP, MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctedv05z: signed int compound divide via stack */
static char const ctedv05z[] = {
	RIGHT, S_STACK,
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	EXL,
	OP, ' ', CRPAIR, ',', POP, '\n',
	MODSWAP, EXRL,
	MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctedv06z: indirect compound divide via stack */
static char const ctedv06z[] = {
	RIGHT, S_STACK,
	LEFT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	EXL,
	OP, ' ', CRPAIR, ',', POP, '\n',
	MODSWAP, EXRL,
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

static struct skeleton const fr_eqdiv[] = {
	{ SU_ADDR | T_UNSN, SU_ADDR | T_INT, ctedv01z },
	{ SU_ADDR | T_CHAR, SU_ADDR | T_INT, ctedv12z },
	{ SU_ADDR | T_INT, SU_ADDR | T_INT, ctedv02z },
	{ SU_ADDR | T_UNSN, SU_EASY | T_ANY, ctedv03z },
	{ SU_ADDR | T_LONG, SU_ADDR | T_INT, ctedv04z },
	{ SU_ADDR | T_CHAR, SU_ANY | T_ANY, ct01vz },
	{ SU_ADDR | T_INT, SU_ANY | T_ANY, ctedv05z },
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, ctedv05z },
	{ SU_ADDR | T_FLOAT, SU_ANY | T_FLOAT, ctreo06z },
	{ SU_ANY | T_ANY | T_INDR, SU_ANY | T_ANY, ctedv06z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_ANY, ctedv06z },
	{ SU_ANY | T_FLOAT | T_INDR, SU_ANY | T_FLOAT, ctreo19z },
	{ 0, 0, NULL }
};

/* fe_eqdiv: compound divide for effect */
static char const ctedv07z[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '\n',
	EXTW, ' ', CR, '\n',
	EXL,
	OP, ' ', CRPAIR, ',', RADDR, '\n',
	MODSWAP, MOV, TLEFT, ' ', LADDR, ',', CR, '\n',
	0
};

static char const ctedv08z[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '\n',
	EXL,
	OP, ' ', CRPAIR, ',', RADDR, '\n',
	MODSWAP, MOV, TLEFT, ' ', LADDR, ',', CR, '\n',
	0
};

static struct skeleton const fe_eqdiv[] = {
	{ SU_ADDR | T_UNSN, SU_ADDR | T_INT, ctedv01z },
	{ SU_REG | T_CHAR, SU_ADDR | T_INT, ctedv07z },
	{ SU_REG | T_INT, SU_ADDR | T_INT, ctedv08z },
	{ 0, 0, NULL }
};

/*
 * fe_eqmod: compound modulus for effect
 * Z8002: remainder already in R(d) after div, no swap needed.
 * MODSWAP is no-op for MOD.
 */
static char const ctedv10z[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '\n',
	EXTW, ' ', CR, '\n',
	EXL,
	OP, ' ', CRPAIR, ',', RADDR, '\n',
	MOV, TLEFT, ' ', LADDR, ',', CR, '\n',
	0
};

static char const ctedv11z[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '\n',
	EXL,
	OP, ' ', CRPAIR, ',', RADDR, '\n',
	MOV, TLEFT, ' ', LADDR, ',', CR, '\n',
	0
};

static struct skeleton const fe_eqmod[] = {
	{ SU_ADDR | T_UNSN, SU_ADDR | T_INT, ctedv01z },
	{ SU_REG | T_CHAR, SU_ADDR | T_INT, ctedv10z },
	{ SU_REG | T_INT, SU_ADDR | T_INT, ctedv11z },
	{ 0, 0, NULL }
};


/* ================================================================
 * COMPOUND SHIFT skeleton group (fr_eqshft=23, fe_eqshft=3)
 *
 * Z8002: sla Rn,#count — dst first, count second (reversed from 68k).
 * ================================================================ */

/* ctesh01z: addressed var shift by 1 */
static char const ctesh01z[] = {
	OP, TLEFT, ' ', LADDR, '-', '\n',
	MOV, TLEFT, ' ', CR, ',', LADDR, '+', '\n',
	0
};

/* ctesh02z: indirect var shift by 1 */
static char const ctesh02z[] = {
	LEFT, S_NEXT | S_INDR,
	OP, TLEFT, ' ', LOFFSET, '(', NAR, ')', '\n',
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	0
};

/* ctesh03z: char var shift by small const (extend, shift, store back) */
static char const ctesh03z[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	EXTW, ' ', CR, '\n',
	OP, ' ', CR, ',', RADDR, '\n',
	MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctesh04z: int/long var shift by small const */
static char const ctesh04z[] = {
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	OP, TLEFT, ' ', CR, ',', RADDR, '\n',
	MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctesh11z: char var shift by variable (extend, shift, store) */
static char const ctesh11z[] = {
	RIGHT, S_NEXT,
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	EXTW, ' ', CR, '\n',
	OP, ' ', CR, ',', NR, '\n',
	MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctesh05z: int/long var shift by variable */
static char const ctesh05z[] = {
	RIGHT, S_NEXT,
	MOV, TLEFT, ' ', CR, ',', LADDR, '-', '\n',
	OP, TLEFT, ' ', CR, ',', NR, '\n',
	MOV, TLEFT, ' ', LADDR, '+', ',', CR, '\n',
	0
};

/* ctesh12z: indirect char var shift by small const */
static char const ctesh12z[] = {
	LEFT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	OP, ' ', CR, ',', RADDR, '\n',
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ctesh06z: indirect int/long var shift by small const */
static char const ctesh06z[] = {
	LEFT, S_NEXT | S_INDR,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	OP, TLEFT, ' ', CR, ',', RADDR, '\n',
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ctesh13z: indirect char var shift by variable */
static char const ctesh13z[] = {
	LEFT, S_NEXT | S_INDR,
	RIGHT, S_NEXT,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	EXTW, ' ', CR, '\n',
	OP, ' ', CR, ',', NR, '\n',
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

/* ctesh07z: indirect int/long var shift by variable */
static char const ctesh07z[] = {
	LEFT, S_NEXT | S_INDR,
	RIGHT, S_NEXT,
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	OP, TLEFT, ' ', CR, ',', NR, '\n',
	MOV, TLEFT, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

static struct skeleton const fr_eqshft[] = {
	{ SU_ADDR | T_INT, SU_ONE | T_ANY, ctesh01z },
	{ SU_ANY | T_INT | T_INDR, SU_ONE | T_ANY, ctesh02z },
	{ SU_ADDR | T_CHAR, SU_SMALL | T_INT, ctesh03z },
	{ SU_ADDR | T_INT, SU_SMALL | T_INT, ctesh04z },
	{ SU_ADDR | T_LONG, SU_SMALL | T_INT, ctesh04z },
	{ SU_ADDR | T_CHAR, SU_ANY | T_ANY, ctesh11z },
	{ SU_ADDR | T_CHAR, SU_ANY | T_LONG, ctesh11z },
	{ SU_ADDR | T_INT, SU_ANY | T_ANY, ctesh05z },
	{ SU_ADDR | T_INT, SU_ANY | T_LONG, ctesh05z },
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, ctesh05z },
	{ SU_ADDR | T_LONG, SU_ANY | T_LONG, ctesh05z },
	{ SU_ANY | T_CHAR | T_INDR, SU_SMALL | T_INT, ctesh12z },
	{ SU_ANY | T_INT | T_INDR, SU_SMALL | T_INT, ctesh06z },
	{ SU_ANY | T_LONG | T_INDR, SU_SMALL | T_INT, ctesh06z },
	{ SU_ANY | T_CHAR | T_INDR, SU_ANY | T_ANY, ctesh13z },
	{ SU_ANY | T_CHAR | T_INDR, SU_ANY | T_LONG, ctesh13z },
	{ SU_ANY | T_INT | T_INDR, SU_ANY | T_ANY, ctesh07z },
	{ SU_ANY | T_INT | T_INDR, SU_ANY | T_LONG, ctesh07z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_ANY, ctesh07z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_LONG, ctesh07z },
	{ 0, 0, NULL }
};

/* fe_eqshft: compound shift for effect */
static char const ctesh08z[] = {
	OP, TLEFT, ' ', LADDR, '\n',
	0
};

static char const ctesh09z[] = {
	LEFT, S_NEXT | S_INDR,
	OP, TLEFT, ' ', LOFFSET, '(', NAR, ')', '\n',
	0
};

static char const ctesh10z[] = {
	RIGHT, 0,
	OP, TLEFT, ' ', LADDR, ',', CR, '\n',
	0
};

static struct skeleton const fe_eqshft[] = {
	{ SU_ADDR | T_INT, SU_ONE | T_ANY, ctesh08z },
	{ SU_ANY | T_INT | T_INDR, SU_ONE | T_ANY, ctesh09z },
	{ SU_REG | T_ANY, SU_SMALL | T_INT, ctasg09 },
	{ SU_REG | T_LONG, SU_SMALL | T_INT, ctasg09 },
	{ SU_REG | T_ANY, SU_ANY | T_ANY, ctesh10z },
	{ SU_REG | T_LONG, SU_ANY | T_ANY, ctesh10z },
	{ 0, 0, NULL }
};


/* ================================================================
 * COMPOUND XOR skeleton group (fr_eqxor=24, fe_eqxor=4)
 * ================================================================ */

/* ctexo01z: addressed var ^= right */
static char const ctexo01z[] = {
	RIGHT, 0,
	EXRL,
	OP, TLEFT, ' ', LADDR, '-', ',', CR, '\n',
	MOV, TLEFT, ' ', CR, ',', LADDR, '+', '\n',
	0
};

/* ctexo02z: indirect var ^= right (via stack) */
static char const ctexo02z[] = {
	LEFT, S_STACK | S_INDR,
	RIGHT, 0,
	EXRL,
	MOVL, ' ', NR, ',', POP, '\n',
	OP, TLEFT, ' ', LOFFSET, '(', NR, ')', ',', CR, '\n',
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NR, ')', '\n',
	0
};

static struct skeleton const fr_eqxor[] = {
	{ SU_ADDR | T_ANY, SU_ANY | T_ANY, ctexo01z },
	{ SU_ADDR | T_ANY, SU_ANY | T_LONG, ctexo01z },
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, ctexo01z },
	{ SU_ADDR | T_LONG, SU_ANY | T_LONG, ctexo01z },
	{ SU_ANY | T_ANY | T_INDR, SU_ANY | T_ANY, ctexo02z },
	{ SU_ANY | T_ANY | T_INDR, SU_ANY | T_LONG, ctexo02z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_ANY, ctexo02z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_LONG, ctexo02z },
	{ 0, 0, NULL }
};

/* fe_eqxor: compound XOR for effect */
static char const ctexo05z[] = {
	MOV, TRIGHT, ' ', CR, ',', RADDR, '\n',
	OP, TLEFT, ' ', LADDR, ',', CR, '\n',
	0
};

static struct skeleton const fe_eqxor[] = {
	{ SU_ADDR | T_ANY, SU_CONST | T_INT, ctasg09 },
	{ SU_ADDR | T_LONG, SU_CONST | T_INT, ctasg09 },
	{ SU_ADDR | T_ANY, SU_ANY | T_ANY, ctasg02 },
	{ SU_ADDR | T_ANY, SU_ANY | T_LONG, ctasg02 },
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, ctasg02 },
	{ SU_ADDR | T_LONG, SU_ANY | T_LONG, ctasg02 },
	{ SU_ANY | T_ANY | T_INDR, SU_EASY | T_ANY, ctasg05z },
	{ SU_ANY | T_ANY | T_INDR, SU_EASY | T_LONG, ctasg05z },
	{ SU_ANY | T_LONG | T_INDR, SU_EASY | T_ANY, ctasg05z },
	{ SU_ANY | T_LONG | T_INDR, SU_EASY | T_LONG, ctasg05z },
	{ SU_ANY | T_ANY | T_INDR, SU_ANY | T_ANY, ctasg06z },
	{ SU_ADDR | T_LONG, SU_CONST | T_INT, ctasg09 },
	{ SU_ADDR | T_LONG, SU_ADDR | T_LONG, ctexo05z },
	{ SU_ADDR | T_LONG, SU_ANY | T_LONG | T_INDR, cteop01z },
	{ SU_ADDR | T_LONG, SU_ANY | T_LONG, ctasg02 },
	{ SU_ANY | T_LONG | T_INDR, SU_CONST | T_INT, ctasg19z },
	{ SU_EASY | T_LONG | T_INDR, SU_ANY | T_LONG, ctasg05z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_LONG, cteop08z },
	{ 0, 0, NULL }
};


/* ================================================================
 * COMPOUND ADDRESS skeleton group (fr_eqaddr=29, fe_eqaddr=5)
 *
 * Z8002: LEA expands to "lda" via strtab.
 * On Z8002 with unified registers, no SU_AREG distinction needed.
 * Pointers are 16-bit (PTRSIZE=2), so use word operations.
 * ================================================================ */

/* cteqa02z: addressed ptr (load, add offset, store back) */
static char const cteqa02z[] = {
	MOV, ' ', CR, ',', LADDR, '\n',
	LEA, ' ', CR, ',', RADDR, '\n',
	MOV, ' ', LADDR, ',', CR, '\n',
	0
};

/* cteqa03z: indirect ptr (load via INDR, add offset, store back) */
static char const cteqa03z[] = {
	LEFT, S_NEXT | S_INDR,
	MOV, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	LEA, ' ', CR, ',', RADDR, '\n',
	MOV, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

static struct skeleton const fr_eqaddr[] = {
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, cteqa02z },
	{ SU_ADDR | T_LONG, SU_ANY | T_LONG, cteqa02z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_ANY, cteqa03z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_LONG, cteqa03z },
	{ 0, 0, NULL }
};

/* fe_eqaddr: compound address for effect */
static char const cteqa05z[] = {
	MOV, ' ', CR, ',', LADDR, '\n',
	LEA, ' ', CR, ',', RADDR, '\n',
	MOV, ' ', LADDR, ',', CR, '\n',
	0
};

static char const cteqa06z[] = {
	LEFT, S_NEXT | S_INDR,
	MOV, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	LEA, ' ', CR, ',', RADDR, '\n',
	MOV, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

static struct skeleton const fe_eqaddr[] = {
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, cteqa05z },
	{ SU_ADDR | T_LONG, SU_ANY | T_LONG, cteqa05z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_ANY, cteqa06z },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_LONG, cteqa06z },
	{ 0, 0, NULL }
};


/* ================================================================
 * COMPLEMENT ASSIGN skeleton group (fr_eqnot=30, fe_eqnot=31)
 * Z8002: com/neg are unary — no operand reversal.
 * ================================================================ */

/* ct76xz: addressed var ~= / neg= */
static char const ct76xz[] = {
	OP, TLEFT, ' ', LADDR, '-', '\n',
	MOV, TLEFT, ' ', CR, ',', LADDR, '+', '\n',
	0
};

/* ct77xz: float ~= via library */
static char const ct77xz[] = {
	LEFT, S_STACK,
	JSR, ' ', OPCALL, '\n',
	POP4, MOV, TLEFT, ' ', LADDR, ',', CR, '\n',
	0
};

/* ct78xz: indirect var ~= / neg= */
static char const ct78xz[] = {
	LEFT, S_NEXT | S_INDR,
	OP, TLEFT, ' ', LOFFSET, '(', NAR, ')', '\n',
	MOV, TLEFT, ' ', CR, ',', LOFFSET, '(', NAR, ')', '\n',
	0
};

/* ct79xz: indirect float ~= via library */
static char const ct79xz[] = {
	LEFT, S_NEXT | S_INDR,
	PSH, LOFFSET, '(', NAR, ')', '\n',
	JSR, ' ', OPCALL, '\n',
	POP4, MOVL, ' ', LOFFSET, '(', NAR, ')', ',', CR, '\n',
	0
};

static struct skeleton const fr_eqnot[] = {
	{ SU_ADDR | T_ANY, SU_ANY | T_ANY, ct76xz },
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, ct76xz },
	{ SU_ADDR | T_FLOAT, SU_ANY | T_ANY, ct77xz },
	{ SU_ANY | T_ANY | T_INDR, SU_ANY | T_ANY, ct78xz },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_ANY, ct78xz },
	{ SU_ANY | T_FLOAT | T_INDR, SU_ANY | T_ANY, ct79xz },
	{ 0, 0, NULL }
};

/* fe_eqnot: complement for effect */
static char const ct80xz[] = {
	OP, TLEFT, ' ', LADDR, '\n',
	0
};

static char const ct81xz[] = {
	LEFT, S_NEXT | S_INDR,
	OP, TLEFT, ' ', LOFFSET, '(', NAR, ')', '\n',
	0
};

static struct skeleton const fe_eqnot[] = {
	{ SU_ADDR | T_ANY, SU_ANY | T_ANY, ct80xz },
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, ct80xz },
	{ SU_ANY | T_ANY | T_INDR, SU_ANY | T_ANY, ct81xz },
	{ SU_ANY | T_LONG | T_INDR, SU_ANY | T_ANY, ct81xz },
	{ 0, 0, NULL }
};


/* ================================================================
 * BIT TEST skeleton group (fc_btst=8)
 *
 * Z8002: bit Rn,#bit / bit offset(Rn),#bit
 * Operand order: Z8002 "bit dst,#bit" (same as 68k "btst #bit,dst"
 * but reversed).
 * ================================================================ */

/* ctbtt01z: bit test addressed var */
static char const ctbtt01z[] = {
	OP, ' ', LADDR, ',', RADDR, '\n',
	0
};

/* ctbtt02z: bit test indirect var */
static char const ctbtt02z[] = {
	LEFT, S_NEXT | S_INDR,
	OP, ' ', LOFFSET, '(', NAR, ')', ',', RADDR, '\n',
	0
};

static struct skeleton const fc_btst[] = {
	{ SU_ADDR | T_ANY, SU_CONST | T_INT, ctbtt01z },
	{ SU_ADDR | T_LONG, SU_CONST | T_INT, ctbtt01z },
	{ SU_ANY | T_ANY | T_INDR, SU_CONST | T_INT, ctbtt02z },
	{ SU_ANY | T_LONG | T_INDR, SU_CONST | T_INT, ctbtt02z },
	{ 0, 0, NULL }
};


/* ================================================================
 * INT TO LONG conversion (fr_itl=26, fs_itl=10)
 * ================================================================ */

/* ctitl01: zero → clear long pair */
static char const ctitl01[] = {
	CLRL, ' ', CR, '\n',
	0
};

/* ctitl02z: unsigned int → long (zero-extend into pair) */
static char const ctitl02z[] = {
	CLR, ' ', CR, '\n',
	MOV, TLEFT, ' ', NR, ',', LADDR, '\n',
	0
};

/* ctitl03z: const int → long (load as long constant) */
static char const ctitl03z[] = {
	MOVL, ' ', CR, ',', LADDR, '\n',
	0
};

/* ctitl04: any int → long (compile, then extend) */
static char const ctitl04[] = {
	LEFT, 0,
	EXL,
	0
};

static struct skeleton const fr_itl[] = {
	{ SU_ZERO | T_ANY, SU_ANY | T_ANY, ctitl01 },
	{ SU_ADDR | T_UNSN, SU_ANY | T_ANY, ctitl02z },
	{ SU_CONST | T_INT, SU_ANY | T_ANY, ctitl03z },
	{ SU_ANY | T_ANY, SU_ANY | T_ANY, ctitl04 },
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, ctshft00 },
	{ 0, 0, NULL }
};

/* fs_itl: int to long on stack */
static char const ctitl05z[] = {
	CLR, ' ', CR, '\n',
	MOV, TLEFT, ' ', NR, ',', LADDR, '\n',
	PSHL, CR, '\n',
	0
};

static char const ctitl06z[] = {
	LEFT, 0,
	EXL,
	PSHL, CR, '\n',
	0
};

/* ct42lz: long to stack (already long, just push) */
static char const ct42lz[] = {
	LEFT, S_STACK,
	0
};

static struct skeleton const fs_itl[] = {
	{ SU_ADDR | T_UNSN, SU_ANY | T_ANY, ctitl05z },
	{ SU_ANY | T_UNSN, SU_ANY | T_ANY, ctitl06z },
	{ SU_ADDR | T_INT, SU_ANY | T_ANY, ctitl06z },
	{ SU_ANY | T_ANY, SU_ANY | T_ANY, ctitl06z },
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, ct42lz },
	{ 0, 0, NULL }
};


/* ================================================================
 * LONG TO INT conversion (fr_lti=27)
 * ================================================================ */

/* ctlti01z: addressed long → int (load long pair, extract low word) */
static char const ctlti01z[] = {
	MOVL, ' ', CRPAIR, ',', LADDR, '\n',
	MOV, ' ', CR, ',', NR, '\n',
	0
};

/* ctlti02z: compiled long → int (compile, extract low word) */
static char const ctlti02z[] = {
	LEFT, 0,
	MOV, ' ', CR, ',', NR, '\n',
	0
};

static struct skeleton const fr_lti[] = {
	{ SU_ADDR | T_LONG, SU_ANY | T_ANY, ctlti01z },
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, ctlti02z },
	{ 0, 0, NULL }
};


/* ================================================================
 * CAST skeleton group (fr_docast=32, fs_docast=33)
 * ================================================================ */

/* ct82xz: cast to unsigned char (mask with $ff — reversed operands) */
static char const ct82xz[] = {
	RIGHT, 0,
	'a', 'n', 'd', ' ', CR, ',', '#', '$', 'f', 'f', '\n',
	0
};

/* ct83xz: cast to signed char (sign-extend byte) */
static char const ct83xz[] = {
	RIGHT, 0,
	EXTW, ' ', CR, '\n',
	0
};

/* ct84x: cast (no-op, just compile right) */
static char const ct84x[] = {
	RIGHT, 0,
	0
};

static struct skeleton const fr_docast[] = {
	{ SU_ANY | T_UCHAR, SU_ANY | T_INT, ct82xz },
	{ SU_ANY | T_UCHAR, SU_ANY | T_LONG, ct82xz },
	{ SU_ANY | T_CHAR, SU_ANY | T_INT, ct83xz },
	{ SU_ANY | T_CHAR, SU_ANY | T_LONG, ct83xz },
	{ SU_ANY | T_ANY, SU_ANY | T_ANY, ct84x },
	{ SU_ANY | T_ANY, SU_ANY | T_LONG, ct84x },
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, ct84x },
	{ SU_ANY | T_LONG, SU_ANY | T_LONG, ct84x },
	{ 0, 0, NULL }
};

/* fs_docast: cast to stack */
static char const ct85xz[] = {
	RIGHT, 0,
	EXRL,
	PSH, CR, '\n',
	0
};

static struct skeleton const fs_docast[] = {
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, ct85xz },
	{ 0, 0, NULL }
};


/* ================================================================
 * FLOAT conversion skeletons (fr_ftol=34, fr_ltof=35, fr_ftoi=36, fr_itof=37)
 * All via library calls.
 * ================================================================ */

static struct skeleton const fr_ftol[] = {
	{ SU_ANY | T_FLOAT, SU_ANY | T_ANY, ctneg03 },
	{ SU_ANY | T_FLOAT | T_INDR, SU_ANY | T_ANY, ctneg03 },
	{ 0, 0, NULL }
};

static struct skeleton const fr_ftoi[] = {
	{ SU_EASY | T_FLOAT, SU_ANY | T_ANY, ctneg03 },
	{ SU_EASY | T_FLOAT | T_INDR, SU_ANY | T_ANY, ctneg03 },
	{ SU_ANY | T_FLOAT, SU_ANY | T_ANY, ctneg03 },
	{ SU_ANY | T_FLOAT | T_INDR, SU_ANY | T_ANY, ctneg03 },
	{ 0, 0, NULL }
};

static struct skeleton const fr_ltof[] = {
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, ctneg03 },
	{ 0, 0, NULL }
};

/* ct48gz: int to float via library (extend to long, push, call) */
static char const ct48gz[] = {
	LEFT, 0,
	EXL,
	PSHL, CR, '\n',
	JSR, ' ', OPCALL, '\n',
	POP4, 0
};

static struct skeleton const fr_itof[] = {
	{ SU_ANY | T_ANY, SU_ANY | T_ANY, ct48gz },
	{ 0, 0, NULL }
};


/* ================================================================
 * TO CHAR conversion (fr_tochar=41)
 * ================================================================ */

/* cttoc01z: unsigned to char (mask with $ff) */
static char const cttoc01z[] = {
	LEFT, 0,
	'a', 'n', 'd', ' ', CR, ',', '#', '$', 'f', 'f', '\n',
	0
};

/* cttoc02: signed to char (sign-extend byte→word) */
static char const cttoc02[] = {
	LEFT, 0,
	EXTW, ' ', CR, '\n',
	0
};

static struct skeleton const fr_tochar[] = {
	{ SU_ANY | T_UNSN, SU_ANY | T_ANY, cttoc01z },
	{ SU_ANY | T_ULONG, SU_ANY | T_ANY, cttoc01z },
	{ SU_ANY | T_ANY, SU_ANY | T_ANY, cttoc02 },
	{ SU_ANY | T_LONG, SU_ANY | T_ANY, cttoc02 },
	{ 0, 0, NULL }
};


/* ================================================================
 * codeskels[] dispatch table
 * ================================================================ */

const struct skeleton *const codeskels[] = {
	0,									/* NULL */
	fe_eqop,							/* 1=FE_EQOP */
	fe_assign,							/* 2=FE_ASSIGN */
	fe_eqshft,							/* 3=FE_EQSHFT */
	fe_eqxor,							/* 4=FE_EQXOR */
	fe_eqaddr,							/* 5=FE_EQADDR */
	fc_fix,								/* 6=FC_FIX */
	fc_rel,								/* 7=FC_REL */
	fc_btst,							/* 8=FC_BTST */
	fs_op,								/* 9=FS_OP */
	fs_itl,								/* 10=FS_ITL */
	fs_ld,								/* 11=FS_LD */
	fr_add,								/* 12=FR_ADD */
	fr_mult,							/* 13=FR_MULT */
	fr_div,								/* 14=FR_DIV */
	fr_shft,							/* 15=FR_SHFT */
	fr_xor,								/* 16=FR_XOR */
	fr_neg,								/* 17=FR_NEG */
	fr_eqop,							/* 18=FR_EQOP */
	fr_postop,							/* 19=FR_POSTOP */
	fr_assign,							/* 20=FR_ASSIGN */
	fr_eqmult,							/* 21=FR_EQMULT */
	fr_eqdiv,							/* 22=FR_EQDIV */
	fr_eqshft,							/* 23=FR_EQSHFT */
	fr_eqxor,							/* 24=FR_EQXOR */
	fr_call,							/* 25=FR_CALL */
	fr_itl,								/* 26=FR_ITL */
	fr_lti,								/* 27=FR_LTI */
	fr_ld,								/* 28=FR_LD */
	fr_eqaddr,							/* 29=FR_EQADDR */
	fr_eqnot,							/* 30=FR_EQNOT */
	fe_eqnot,							/* 31=FE_EQNOT */
	fr_docast,							/* 32=FR_DOCAST */
	fs_docast,							/* 33=FS_DOCAST */
	fr_ftol,							/* 34=FR_FTOL */
	fr_ltof,							/* 35=FR_LTOF */
	fr_ftoi,							/* 36=FR_FTOI */
	fr_itof,							/* 37=FR_ITOF */
	fe_eqmult,							/* 38=FE_EQMULT */
	fe_eqdiv,							/* 39=FE_EQDIV */
	fe_eqmod,							/* 40=FE_EQMOD */
	fr_tochar,							/* 41=FR_TOCHAR */
	fr_ldiv,							/* 42=FR_LDIV */
};
