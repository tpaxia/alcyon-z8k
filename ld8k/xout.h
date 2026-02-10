/*
 * x.out format definitions for Z8000.
 *
 * Ported for cross-compilation on macOS:
 *   - Uses fixed-width types (stdint.h) to match on-disk layout
 *   - Packed structs to prevent padding
 *   - Byte-swap helpers for big-endian file format on little-endian host
 */

#ifndef XOUT_H
#define XOUT_H

#include <stdint.h>
#include <arpa/inet.h>

/*
 * All multi-byte fields in x.out are big-endian (Z8000 native order).
 * On little-endian hosts we must swap after read and before write.
 */

struct x_hdr {
	uint16_t	x_magic;	/* magic number */
	int16_t		x_nseg;		/* number of segments in file */
	int32_t		x_init;		/* length of initialized part of file */
	int32_t		x_reloc;	/* length of relocation part of file */
	int32_t		x_symb;		/* length of symbol table part of file */
} __attribute__((packed));


struct x_sg {
	uint8_t		x_sg_no;	/* assigned number of segment */
	uint8_t		x_sg_typ;	/* type of segment */
	uint16_t	x_sg_len;	/* length of segment */
} __attribute__((packed));


#define X_SU_MAGIC	0xEE00	/* segmented, non executable */
#define	X_SX_MAGIC	0xEE01	/* segmented, executable */
#define X_NU_MAGIC	0xEE02	/* non-segmented, non executable */
#define X_NXN_MAGIC	0xEE03	/* non-segmented, executable, non-shared */
#define X_NUS_MAGIC	0xEE06	/* non-segmented, non executable, shared */
#define X_NXS_MAGIC	0xEE07	/* non-segmented, executable, shared */
#define X_NUI_MAGIC	0xEE0A	/* non-segmented, non executable, split ID */
#define X_NXI_MAGIC	0xEE0B	/* non-segmented, executable, split ID */

#define X_SG_BSS	1	/* non-initialized data segment */
#define X_SG_STK	2	/* stack segment, no data in file */
#define X_SG_COD	3	/* code segment */
#define X_SG_CON	4	/* constant pool */
#define X_SG_DAT	5	/* initialized data */
#define X_SG_MXU	6	/* mixed code and data, not protectable */
#define X_SG_MXP	7	/* mixed code and data, protectable */

struct x_rel {			/* relocation item */
	uint8_t		x_rl_sgn;	/* segment containing item to be relocated */
	uint8_t		x_rl_flg;	/* relocation type (see below) */
	uint16_t	x_rl_loc;	/* location of item to be relocated */
	uint16_t	x_rl_bas;	/* number of (external) element in symbol table
				          or (internal) segment by which to relocate */
} __attribute__((packed));


#define X_RL_OFF	1	/* adjust a 16 bit offset value only */
#define X_RL_SSG	2	/* adjust a short form segment plus offset */
#define X_RL_LSG	3	/* adjust a long form (32 bit) seg plus off */
#define X_RL_XOF	5	/* adjust a 16 bit offset by an external */
#define X_RL_XSSG	6	/* adjust a short seg ref by an external */
#define X_RL_XLSG	7	/* adjust a long seg ref by an external */

#define XNAMELN	8		/* length of a symbol */

struct x_sym {
	uint8_t		x_sy_sg;	/* the segment number */
	uint8_t		x_sy_fl;	/* the type of entry */
	uint16_t	x_sy_val;	/* the value of this entry */
	char		x_sy_name[XNAMELN];	/* the symbol name, padded with 0's */
} __attribute__((packed));

#define X_SY_LOC	1	/* local symbol (for debug only) */
#define X_SY_UNX	2	/* undefined external entry */
#define X_SY_GLB	3	/* global definition */
#define X_SY_SEG	4	/* segment name */
#define X_SY_IND	5	/* indirection: use value */


/*
 * Byte-swap helpers.  x.out is big-endian; these swap after read / before write.
 * ntohs/htons are involutions, so the same function works both ways.
 */

static inline void swap_hdr(struct x_hdr *h)
{
	h->x_magic = ntohs(h->x_magic);
	h->x_nseg  = ntohs(h->x_nseg);
	h->x_init  = ntohl(h->x_init);
	h->x_reloc = ntohl(h->x_reloc);
	h->x_symb  = ntohl(h->x_symb);
}

static inline void swap_sg(struct x_sg *s)
{
	s->x_sg_len = ntohs(s->x_sg_len);
}

static inline void swap_rel(struct x_rel *r)
{
	r->x_rl_loc = ntohs(r->x_rl_loc);
	r->x_rl_bas = ntohs(r->x_rl_bas);
}

static inline void swap_sym(struct x_sym *s)
{
	s->x_sy_val = ntohs(s->x_sy_val);
}

/*
 * Big-endian 16-bit access helpers for code data buffers.
 */
static inline uint16_t rd16be(const char *p)
{
	return ((unsigned char)p[0] << 8) | (unsigned char)p[1];
}

static inline void wr16be(char *p, uint16_t val)
{
	p[0] = (val >> 8) & 0xff;
	p[1] = val & 0xff;
}

/*
 * Archive format.
 */
#define AR8KMAGIC	0177545		/* 0xFF65 */
#define AR8KMAGIC_BE	0x65FF		/* byte-swapped on LE host */

struct ar8k_hd {
	char		ar8k_name[14];
	int32_t		ar8k_date;
	uint8_t		ar8k_uid;
	uint8_t		ar8k_gid;
	int16_t		ar8k_mode;
	int32_t		ar8k_size;
} __attribute__((packed));

static inline void swap_ar(struct ar8k_hd *a)
{
	a->ar8k_date = ntohl(a->ar8k_date);
	a->ar8k_mode = ntohs(a->ar8k_mode);
	a->ar8k_size = ntohl(a->ar8k_size);
}

#endif /* XOUT_H */
