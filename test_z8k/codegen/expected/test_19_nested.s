	.global _x
_x	.common
	.block 2
	.global _y
_y	.common
	.block 2
	.global _z
_z	.common
	.block 2
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 6
	ld R0,_y
	add R0,_z
	ld R1,_y
	sub R1,_z
	ld R0,R1
	mult RR0,R0
	ld R0,R1
	ld _x,R0
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
; line 12
	ld R0,4(R14)
	cp R0,6(R14)
	jr le, L3
; line 13
; line 14
	ld R0,6(R14)
	cp R0,8(R14)
	jr le, L4
; line 14
	ld R0,4(R14)
	jp L2
L4:

; line 15
	ld R0,6(R14)
	jp L2
; line 16
L3:

; line 17
	ld R0,8(R14)
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
