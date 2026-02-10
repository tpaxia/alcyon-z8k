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
	ld _x,#12
; line 7
	ld R0,_x
	ld R1,R0
	mult RR0,_x
	ld R0,R1
	ld _y,R0
; line 8
	ld R0,_y
	ld R1,R0
	exts RR0
	div RR0,#3
	ld R0,R1
	ld _x,R0
; line 9
	ld R0,_x
	ld R1,R0
	exts RR0
	div RR0,#3
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
