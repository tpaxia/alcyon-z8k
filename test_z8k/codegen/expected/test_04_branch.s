	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 5
; line 6
	tst 4(R14)
	jr le, L2
; line 6
	ld R0,#1
	jp L1
L2:

; line 7
	clr R0
	jp L1
L1:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
