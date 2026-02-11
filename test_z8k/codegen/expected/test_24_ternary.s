	.global _a
_a	.common
	.block 2
	.global _b
_b	.common
	.block 2
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 6
	ld _a,#5
; line 7
	ld _b,#3
; line 8
	ld R0,_a
	cp R0,_b
	jr le, L10000
	ld R0,_a
	jp L10001
L10000:
	ld R0,_b
L10001:
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

; line 13
	tst _a
	jr ne, L10002
	ld R0,#1
	jp L10003
L10002:
	clr R0
L10003:
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
