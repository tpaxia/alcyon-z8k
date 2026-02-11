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
	ld R0,_ix
	ld R1,R0
	exts RR0
	ldl _lx,RR0
; line 9
	ldl RR0,_lx
	ld R0,R1
	ld _ix,R0
; line 10
	ld R0,_ix
	ldb _cx,R0
; line 11
	ldb RL0,_cx
	extsb R0
	jp L1
L1:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
