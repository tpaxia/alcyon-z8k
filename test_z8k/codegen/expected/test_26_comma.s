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
	ld R0,#1
	ld R0,#2
	ld R0,#3
	ld _x,R0
; line 7
	ld R0,_x
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

; line 12
	clr _x
; line 13
	clr _y
; line 14
	ld _y,#5
	ld R0,_y
	add R0,#1
	ld _x,R0
; line 15
	ld R0,_x
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
