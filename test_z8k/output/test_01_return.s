	.global _main
__text	.sect
_main:

	push @R15,R14
	ld R14,R15
	add R15,#-4
; line 4
	ld R0,#42
	jp L1
L1:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
