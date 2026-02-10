;*******************************************************
;* lmul - 32-bit long multiply
;*
;*  Called with two 32-bit longs on stack.
;*  Returns lower 32 bits of product in RR0.
;*
;*  Uses Z8000 MULTL instruction (32x32 -> 64).
;*  Lower 32 bits are identical for signed/unsigned.
;*
;*  Stack at entry:
;*    SP+0: return address (2)
;*    SP+2: arg1 (4)
;*    SP+6: arg2 (4)
;*******************************************************

	.global lmul
	.global _lmul

__text	.sect

lmul:
_lmul:
	ldl	RR2, 2(R15)	; arg1 -> lower half of RQ0
	ldl	RR4, 6(R15)	; arg2
	multl	RQ0, RR4	; RQ0 = RR2 * RR4 (64-bit)
	ld	R0, R2		; copy lower 32 bits to RR0
	ld	R1, R3
	ret

	.end
