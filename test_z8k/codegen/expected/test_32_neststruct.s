	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15
	add R15,#-8
; line 15
	ld R1,R14
	ld -8(R1),#1
; line 16
	ld R1,R14
	ld -6(R1),#2
; line 17
	ld R1,R14
	ld -4(R1),#10
; line 18
	ld R1,R14
	ld -2(R1),#20
; line 19
	ld R0,R14
	ld R0,-4(R0)
	ld R1,R14
	ld R1,-8(R1)
	sub R0,R1
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

; line 25
	ld R0,4(R14)
	ld R0,6(R0)
	ld R1,4(R14)
	ld R1,2(R1)
	sub R0,R1
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
