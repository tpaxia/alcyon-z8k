	.global _lx
_lx	.common
	.block 4
	.global _ix
_ix	.common
	.block 2
	.global _cx
_cx	.common
	.block 2
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 8
	ld _ix,#100
; line 9
	ld R0,_ix
	add R0,#1
	ld R1,R0
	exts RR0
	ldl _lx,RR0
; line 10
	ldl RR0,_lx
	ld R0,R1
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

; line 15
	ldb _cx,#10
; line 16
	ldb RL0,_cx
	extsb R0
	extsb R0
	add R0,#5
	ld _ix,R0
; line 17
	ld R0,_ix
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

; line 22
	ldl _lx,#$3e8
; line 23
	ld _ix,#50
; line 24
	ld R0,_ix
	ld R1,R0
	exts RR0
	addl RR0,_lx
	ldl _lx,RR0
; line 25
	ldl RR0,_lx
	ld R0,R1
	ld _ix,R0
; line 26
	ld R0,_ix
	jp L3
L3:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
