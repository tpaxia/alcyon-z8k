	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15
	add R15,#-2
; line 5
	clr -2(R14)
; line 6
	jp L4
L3:

; line 7
; line 8
	cp -2(R14),#5
	jr eq, L2
; line 9
	ld R0,-2(R14)
	add R0,#1
	ld -2(R14),R0
L4:

; line 10
	jp L3
L2:

; line 11
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
	add R15,#-4
; line 18
	clr -4(R14)
; line 19
	clr -2(R14)
; line 20
	jp L8
L7:

; line 21
	ld R0,-2(R14)
	add R0,#1
	ld -2(R14),R0
; line 22
; line 23
	cp -2(R14),#5
	jr gt, L8
; line 24
	ld R0,-4(R14)
	add R0,-2(R14)
	ld -4(R14),R0
L8:

; line 25
	cp -2(R14),#10
	jr lt, L7
L6:

; line 26
	ld R0,-4(R14)
	jp L5
L5:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
