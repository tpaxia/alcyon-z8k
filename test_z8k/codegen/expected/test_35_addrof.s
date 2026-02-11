	.global _arr
_arr	.common
	.block 10
	.global _add
__text	.sect
_add:

	push @R15,R14
	ld R14,R15

; line 7
	ld R0,4(R14)
	add R0,6(R14)
	jp L1
L1:
	ld R15,R14
	pop R14,@R15
	ret
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15
	add R15,#-6
; line 14
	ld -2(R14),#42
; line 15
	ld R0,R14
	sub R0,#2
	ld -6(R14),R0
; line 16
	ld R1,-6(R14)
	ld R0,(R1)
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
	.global _g
__text	.sect
_g:

	push @R15,R14
	ld R14,R15
	add R15,#-4
; line 22
	ld -4(R14),#4+_arr
; line 23
	ld R1,-4(R14)
	ld (R1),#77
; line 24
	ld R0,4+_arr
	jp L3
L3:
	ld R15,R14
	pop R14,@R15
	ret
	.global _h
__text	.sect
_h:

	push @R15,R14
	ld R14,R15
	add R15,#-4
; line 30
	ld -4(R14),#_add
; line 31
	push @R15,#4
	push @R15,#3
	ld R1,-4(R14)
	call (R1)
	inc R15,#4
	jp L4
L4:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
