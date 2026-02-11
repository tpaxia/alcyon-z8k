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
	ld R0,#5
	ld _c,R0
	ld _b,R0
	ld _a,R0
; line 7
	ld R0,_a
	add R0,_b
	add R0,_c
	jp L1
L1:
	ld R15,R14
	pop R14,@R15
	ret
	.global _g
__text	.sect
_g:

	push @R15,R14
	ld R14,R15
	add R15,#-4
; line 13
	ld R0,#10
	ld -4(R14),R0
	ld -2(R14),R0
; line 14
	ld R0,-2(R14)
	add R0,-4(R14)
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
