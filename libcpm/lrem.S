;*******************************************************
;* lrem - 32-bit signed long remainder
;*
;*  Called with two 32-bit longs on stack.
;*  Returns remainder in RR0.
;*
;*  Uses Z8000 DIVL instruction (64/32 -> 32q,32r).
;*  Remainder lands directly in RR0.
;*
;*  Stack at entry:
;*    SP+0: return address (2)
;*    SP+2: dividend (4)
;*    SP+6: divisor (4)
;*******************************************************

	.global lrem
	.global _lrem

__text	.sect

lrem:
_lrem:
	ldl	RR2, 2(R15)	; dividend -> lower half of RQ0
	ld	R0, R2		; sign-extend: copy high word
	sra	R0, #15		; R0 = 0x0000 or 0xFFFF
	ld	R1, R0		; RR0 = sign extension
	ldl	RR4, 6(R15)	; divisor
	divl	RQ0, RR4	; quotient in RR2, remainder in RR0
	ret			; RR0 already has remainder

	.end
