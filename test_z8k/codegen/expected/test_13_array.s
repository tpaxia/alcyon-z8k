	.global _arr
_arr	.common
	.block 20
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 6
	ld _arr,#1
; line 7
	ld 10+_arr,#2
; line 8
	ld _arr,10+_arr
L1:
	ld R15,R14
	pop R14,@R15
	ret
	.global _g
__text	.sect
_g:

	push @R15,R14
	ld R14,R15

; line 14
	ld R0,4(R14)
	sla R0,#1
	ld R1,R0
	exts RR0
	ld R1,#_arr
	ld R0,0(R0)
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
