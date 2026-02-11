	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

.bss
L2:
.ds.b 2
__text	.sect
; line 5
	ld R0,L2
	add R0,#1
	ld L2,R0
; line 6
	ld R0,L2
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

__data	.sect
L4:
	.word	0Ah
__text	.sect
; line 12
	ld R0,L4
	add R0,#5
	ld L4,R0
; line 13
	ld R0,L4
	jp L3
L3:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
