	.global _pt
_pt	.common
	.block 4
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 11
	ld _pt,#10
; line 12
	ld 2+_pt,#20
; line 13
	ld R0,_pt
	add R0,2+_pt
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

; line 19
	ld R1,4(R14)
	ld (R1),#5
; line 20
	ld R0,4(R14)
	ld R0,2(R0)
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
