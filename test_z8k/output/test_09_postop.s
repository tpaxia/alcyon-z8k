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
	add R15,#-4
; line 6
	clr _x
; line 7
	ld _y,_x
	add _x,#1
; line 8
	add _x,#1
	ld _y,_x
; line 9
	ld _y,_x
	sub _x,#1
; line 10
	ld R0,_y
	jp L1
L1:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
