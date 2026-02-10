;*******************************************************
;* ldiv - 32-bit signed long divide
;*
;*  Called with two 32-bit longs on stack.
;*  Returns quotient in RR0.
;*  Stores remainder in _ldivr (for lrem compat).
;*
;*  Uses Z8000 DIVL instruction (64/32 -> 32q,32r).
;*  Dividend is sign-extended to 64 bits in RQ0.
;*
;*  Stack at entry:
;*    SP+0: return address (2)
;*    SP+2: dividend (4)
;*    SP+6: divisor (4)
;*******************************************************

	.global ldiv
	.global _ldiv
	.global _ldivr

__bss	.sect

_ldivr:	.block	4

__text	.sect

ldiv:
_ldiv:
	ldl	RR2, 2(R15)	; dividend -> lower half of RQ0
	ld	R0, R2		; sign-extend: copy high word
	sra	R0, #15		; R0 = 0x0000 or 0xFFFF
	ld	R1, R0		; RR0 = sign extension
	ldl	RR4, 6(R15)	; divisor
	divl	RQ0, RR4	; quotient in RR2, remainder in RR0
	ld	_ldivr, R0	; store remainder (high word)
	ld	_ldivr+2, R1	; store remainder (low word)
	ld	R0, R2		; copy quotient to RR0
	ld	R1, R3
	ret

	.end
