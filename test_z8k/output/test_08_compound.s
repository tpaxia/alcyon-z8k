	.global _x
_x	.common
	.block 2
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15
	add R15,#-4
; line 6
	ld _x,#10
; line 7
	add _x,#5
; line 8
	sub _x,#3
; line 9
	and _x,#7
; line 10
	or _x,#16
; line 11
	ld R0,_x
	jp L1
L1:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
