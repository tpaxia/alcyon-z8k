	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

	push @R15,R7
	push @R15,R6
	push @R15,R5
; line 6
	ld R7,#10
; line 7
	ld R6,#20
; line 8
	ld R0,R7
	add R0,R6
	jp L1
L1:
	add R15,#4
	pop R6,@R15
	pop R7,@R15
	ld R15,R14
	pop R14,@R15
	ret
	.global _g
__text	.sect
_g:

	push @R15,R14
	ld R14,R15

	push @R15,R7
	push @R15,R6
	push @R15,R5
; line 16
	clr R7
; line 17
	ld R6,#1
; line 18
	jp L5
L4:

; line 19
	add R7,R6
; line 20
	add R6,#1
L5:

; line 21
	cp R6,4(R14)
	jr le, L4
L3:

; line 22
	ld R0,R7
	jp L2
L2:
	add R15,#4
	pop R6,@R15
	pop R7,@R15
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
