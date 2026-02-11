	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15
	add R15,#-6
; line 5
	ld -2(R14),#10
; line 6
	ld -4(R14),#20
; line 7
	ld R0,-2(R14)
	add R0,-4(R14)
	ld -6(R14),R0
; line 8
	ld R0,-6(R14)
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
	add R15,#-2
; line 15
	ld -2(R14),4(R14)
; line 16
	ld 4(R14),6(R14)
; line 17
	ld 6(R14),-2(R14)
; line 18
	ld R0,4(R14)
	add R0,6(R14)
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
