	.global _grid
_grid	.common
	.block 24
	.global _f
__text	.sect
_f:

	push @R15,R14
	ld R14,R15

; line 6
	ld _grid,#1
; line 7
	ld 12+_grid,#5
; line 8
	ld 22+_grid,#9
; line 9
	ld R0,12+_grid
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

; line 15
	ld R1,4(R14)
	sla R1,#3
	ld R1,R1
	exts RR0
	ld R2,6(R14)
	sla R2,#1
	ld R3,R2
	exts RR2
	addl R1,R2
	add R1,#_grid
	ld (R1),#42
; line 16
	ld R0,4(R14)
	sla R0,#3
	ld R1,R0
	exts RR0
	ld R1,6(R14)
	sla R1,#1
	ld R1,R1
	exts RR0
	addl R0,R1
	ld R1,#_grid
	ld R0,0(R0)
	jp L2
L2:
	ld R15,R14
	pop R14,@R15
	ret
__data	.sect
	.end
