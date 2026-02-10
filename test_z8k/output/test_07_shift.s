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
	ld R0,_y
	sla R0,#2
	ld _x,R0
; line 7
	ld R0,_x
	sra R0,#1
	ld _x,R0
; line 8
	ld R0,_x
	ld R1,_y
	xor R0,R1
	ld _x,R0
; line 9
	ld R0,_x
	com R0
	ld _x,R0
; line 10
	ld R0,_x
	jp L1
L1:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
