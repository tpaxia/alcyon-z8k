	.global _arr
_arr	.common
	.block 10
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 10
	ld _arr,#10
; line 11
	add _arr,#1
; line 12
	ld R0,_arr
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

; line 17
	ld 2+_arr,#5
; line 18
	add 2+_arr,#1
; line 19
	ld R0,2+_arr
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
	.global _h
__text	.sect
_h:

	push @R15,R14
	ld R14,R15

; line 25
	ld R1,4(R14)
	ld R0,(R1)
	ld R2,4(R14)
	add (R2),#1
; line 26
	ld R1,4(R14)
	ld R0,(R1)
	jp L3
L3:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
