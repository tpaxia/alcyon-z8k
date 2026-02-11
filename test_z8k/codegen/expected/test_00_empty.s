	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

L1:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
