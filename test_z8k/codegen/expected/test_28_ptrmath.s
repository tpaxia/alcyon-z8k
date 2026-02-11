	.global _arr
_arr	.common
	.block 10
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15
	add R15,#-4
; line 7
	ld -4(R14),#_arr
; line 8
	ld R1,-4(R14)
	ld (R1),#10
; line 9
	ld R0,-4(R14)
	add R0,#2
	ld -4(R14),R0
; line 10
	ld R1,-4(R14)
	ld (R1),#20
; line 11
	ld R0,-4(R14)
	ld R0,-2(R0)
	ld R2,-4(R14)
	ld R1,(R2)
	add R0,R1
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
	add R15,#-8
; line 18
	ld -4(R14),#_arr
; line 19
	ld -8(R14),#6+_arr
; line 20
	ld R0,-8(R14)
	sub RR0,-4(R14)
	div RR0,#2
	ld R0,R1
	ld R1,R0
	exts RR0
	ld R0,R1
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
