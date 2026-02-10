	.global _x
_x	.common
	.block 2
	.global _main
__text	.sect
_main:

	push @R15,R14
	ld R14,R15
	add R15,#-4
; line 6
	ld _x,#42
L1:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
