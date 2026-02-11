	.global _la
_la	.common
	.block 4
	.global _lb
_lb	.common
	.block 4
	.global _lc
_lc	.common
	.block 4
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 6
	ldl RR0,_la
	addl RR0,_lb
	ldl _lc,RR0
; line 7
	ldl RR0,_la
	subl RR0,_lb
	ldl _lc,RR0
L1:
	ld R15,R14
	pop R14,@R15
	ret
	.global _g
__text	.sect
_g:

	push @R15,R14
	ld R14,R15

; line 12
	pushl @R15,_lb
	pushl @R15,_la
	call lmul
	inc R15,#4
	ldl _lc,RR0
L2:
	ld R15,R14
	pop R14,@R15
	ret
	.global _h
__text	.sect
_h:

	push @R15,R14
	ld R14,R15

; line 17
	pushl @R15,_lb
	pushl @R15,_la
	call ldiv
	inc R15,#4
	ldl _lc,RR0
L3:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
