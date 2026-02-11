	.global _cx
_cx	.common
	.block 2
	.global _cy
_cy	.common
	.block 2
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 6
	ldb _cx,#65
; line 7
	ldb RL0,_cx
	extsb R0
	extsb R0
	add R0,#1
	ldb _cy,R0
; line 8
	ldb RL0,_cy
	extsb R0
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

; line 13
	ldb _cx,#122
; line 14
; line 15
	cpb _cx,#97
	jr lt, L3
	cpb _cx,#122
	jr gt, L3
; line 15
	ld R0,#1
	jp L2
L3:

; line 16
	clr R0
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

; line 21
	ldb _cx,#65
; line 22
	ldb RL0,_cx
	extsb R0
	extsb R0
	add R0,#32
	ldb _cx,R0
; line 23
	ldb RL0,_cx
	extsb R0
	jp L4
L4:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
