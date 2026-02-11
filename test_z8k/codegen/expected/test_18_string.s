	.global _msg
_msg	.common
	.block 4
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 6
	ld _msg,#L2
L1:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
L2:
	.byte	068h,065h,06Ch,06Ch,06Fh,00h
	.end
