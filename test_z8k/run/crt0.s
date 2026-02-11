	.global _main
__text	.sect
; Z8002 reset vector (loaded at address 0x0000)
	.word 0		; 0x0000: reserved
	.word 04000h	; 0x0002: FCW (system mode, non-segmented)
	.word _start	; 0x0004: PC (entry point)
_start:
	ld R15,#0FFFEh	; stack pointer at top of 64K RAM
	ldk R14,#0	; clear frame pointer
	call _main	; call C main() — return value in R0
			; (blank line: asz8k eats one line after call with external symbol)
	.word 07A00h	; halt — stop, emulator reports registers
__data	.sect
	.end
