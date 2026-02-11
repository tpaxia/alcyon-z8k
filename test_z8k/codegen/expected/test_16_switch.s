	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 5
	ld R0,4(R14)
	jp L3
L4:

; line 6
; line 7
	ld R0,#10
	jp L1
L5:

; line 8
; line 9
	ld R0,#20
	jp L1
L6:

; line 10
; line 11
	ld R0,#30
	jp L1
L7:

; line 12
; line 13
	clr R0
	jp L1
	jp L2
L3:
	cp R0,#1
	jr eq,L4
	cp R0,#2
	jr eq,L5
	cp R0,#3
	jr eq,L6
	jp L7
L2:
L1:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
