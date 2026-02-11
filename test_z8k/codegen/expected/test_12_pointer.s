	.global _x
_x	.common
	.block 2
	.global _p
_p	.common
	.block 4
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 7
	ld _p,#_x
; line 8
	ld R1,_p
	ld (R1),#42
; line 9
	ld R1,_p
	ld R0,(R1)
	add R0,#1
	ld _x,R0
L1:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
