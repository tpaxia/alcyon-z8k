	.global _x
_x	.common
	.block 2
	.global _y
_y	.common
	.block 2
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 6
; line 7
	clr R0
	ld R0,_x
	cp R0,_y
	jr ule, L2
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
	clr R0
	ld R0,_x
	srl R0,#2
	jp L3
L3:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
