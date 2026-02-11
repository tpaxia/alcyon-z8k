	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15
	add R15,#-2
; line 9
	ld -2(R14),#1
; line 10
; line 11
	cp -2(R14),#2
	jr ne, L2
; line 11
	ld R0,#1
	jp L1
L2:

; line 12
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
	add R15,#-2
; line 18
	clr -2(R14)
; line 19
	ld R0,-2(R14)
	add R0,#1
	ld -2(R14),R0
; line 20
	ld R0,-2(R14)
	jp L3
L3:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
