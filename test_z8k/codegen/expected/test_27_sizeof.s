	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15
	add R15,#-2
; line 10
	ld -2(R14),#2
; line 11
	ld R0,-2(R14)
	add R0,#1
	ld -2(R14),R0
; line 12
	ld R0,-2(R14)
	add R0,#4
	ld -2(R14),R0
; line 13
	ld R0,-2(R14)
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

; line 18
	ld R0,#4
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
