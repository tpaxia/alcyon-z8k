	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 5
	ld R0,4(R14)
	jp L3
L4:

; line 6
	ld R0,#10
	jp L1
L5:

; line 7
	ld R0,#11
	jp L1
L6:

; line 8
	ld R0,#12
	jp L1
L7:

; line 9
	ld R0,#13
	jp L1
L8:

; line 10
	ld R0,#14
	jp L1
L9:

; line 11
	clr R0
	jp L1
	jp L2
L3:
	cp R0,#4
	jr ugt,L9
	sla R0,#2
	ld R8,R0
	add R8,#L10
	ld R8,(R8)
	jp @R8
__data	.sect
L10:
	.long	L4
	.long	L5
	.long	L6
	.long	L7
	.long	L8
__text	.sect
L2:
L1:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
