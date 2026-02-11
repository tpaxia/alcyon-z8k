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
; line 7
	tst _a
	jr eq, L2
	tst _b
	jr eq, L2
; line 7
	ld R0,#1
	jp L1
L2:

; line 8
	clr R0
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
; line 14
	tst _a
	jr ne, L10000
	tst _b
	jr eq, L4
L10000:
; line 14
	ld R0,#1
	jp L3
L4:

; line 15
	clr R0
	jp L3
L3:
	ld R15,R14
	pop R14,@R15
	ret
	.global _h
__text	.sect
_h:

	push @R15,R14
	ld R14,R15

; line 20
	tst _a
	jr eq, L10001
	clr R0
	jp L10002
L10001:
	ld R0,#1
L10002:
	jp L5
L5:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
