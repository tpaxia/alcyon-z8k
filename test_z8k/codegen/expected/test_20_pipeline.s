	.global _result
_result	.common
	.block 2
	.global _add
__text	.sect
_add:

	push @R15,R14
	ld R14,R15

; line 7
	ld R0,4(R14)
	add R0,6(R14)
	jp L1
L1:
	ld R15,R14
	pop R14,@R15
	ret
	.global _main
__text	.sect
_main:

	push @R15,R14
	ld R14,R15

; line 12
	.global _add
	push @R15,#4
	push @R15,#3
	call _add
	inc R15,#4
	ld _result,R0
; line 13
	ld R0,_result
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
