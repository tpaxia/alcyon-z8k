	.global _a
_a	.common
	.block 2
	.global _b
_b	.common
	.block 2
	.global _c
_c	.common
	.block 2
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 6
	ld R0,_a
	add R0,_b
	ld _c,R0
; line 7
	ld R0,_a
	sub R0,_b
	ld _c,R0
L1:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
