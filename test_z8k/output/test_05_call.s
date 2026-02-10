	.global _add
__text	.sect
_add:

	push @R15,R14
	ld R14,R15
	add R15,#-4
; line 5
	ld R0,8(R14)
	add R0,10(R14)
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
	add R15,#-4
; line 10
	.global _add
	ld @R15,#4
	push @R15,#3
	call _add
	inc R15,#2
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
