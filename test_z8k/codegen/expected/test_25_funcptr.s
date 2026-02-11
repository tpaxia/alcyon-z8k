	.global _add
__text	.sect
_add:

	push @R15,R14
	ld R14,R15

; line 5
	ld R0,4(R14)
	add R0,6(R14)
	jp L1
L1:
	ld R15,R14
	pop R14,@R15
	ret
	.global _call_fn
__text	.sect
_call_fn:

	push @R15,R14
	ld R14,R15

; line 12
	push @R15,10(R14)
	push @R15,8(R14)
	ld R1,4(R14)
	call (R1)
	inc R15,#4
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
	.global _main
__text	.sect
_main:

	push @R15,R14
	ld R14,R15
	add R15,#-4
; line 18
	ld -4(R14),#_add
; line 19
	.global _call_fn
	push @R15,#20
	push @R15,#10
	push @R15,-4(R14)
	call _call_fn
	inc R15,#6
	jp L3
L3:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
