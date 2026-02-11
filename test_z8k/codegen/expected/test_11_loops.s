	.global _sum
_sum	.common
	.block 2
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15
	add R15,#-2
; line 7
	clr _sum
; line 8
	clr -2(R14)
; line 9
	jp L4
L3:

; line 10
	ld R0,-2(R14)
	add _sum,R0
; line 11
	add -2(R14),#1
L4:

; line 12
	cp -2(R14),#10
	jr lt, L3
L2:
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
	ld -2(R14),#5
; line 19
L8:

; line 20
	sub -2(R14),#1
L7:

; line 21
	tst -2(R14)
	jr gt, L8
L6:
L5:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
