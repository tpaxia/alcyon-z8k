;*******************************************************
;*
;*  C runtime startup for CP/M-8000 (Z8002 non-segmented)
;*
;*  Adapted from Digital Research CP/M-Z8K startup
;*  for Alcyon C calling convention:
;*    - Return value in R0 (int) or RR0 (long)
;*    - Frame pointer R14, stack pointer R15
;*    - Args on stack, leftmost at lowest offset
;*
;*******************************************************

;*******************************************************
;* Globals
;*******************************************************

	.global __start
	.global m.init
	.global __exit
	.global _brk
	.global ___BDOS

	.global __break
	.global ___cpmrv
	.global __base
	.global __sovf

	.global ___pname
	.global ___tname
	.global ___lname
	.global ___xeof

;*******************************************************
;* Externals
;*******************************************************

	.global _main

;*******************************************************
;* Definitions
;*******************************************************

				; Offsets in basepage
lbss	.equ	24		; Pointer to bss start
bsslen	.equ	28		; Bss length
command	.equ	128		; Command tail

BDOS_SC	.equ	2		; BDOS system call number
EXIT	.equ	0		; BDOS exit request
PRINT	.equ	9		; BDOS print string

safety	.equ	0100h		; Stack overflow margin

PCSIZE	.equ	2		; PC size (non-segmented)
INTSIZE	.equ	2		; INT data type size

ARG1	.equ	PCSIZE		; First argument offset
ARG2	.equ	ARG1+INTSIZE	; Second argument offset

;*******************************************************
;* Data area
;*******************************************************

__data	.sect

___pname:	.byte	"C runtime ", 0
___tname:	.byte	"CON:", 0
___lname:	.byte	"LST:", 0
___xeof:	.byte	01ah
ovf:		.byte	"Stack Overflow $", 0

__bss	.sect

__break:	.block	2		; End of BSS / heap start
___cpmrv:	.block	2		; Last BDOS return value
__base:		.block	2		; Pointer to basepage

__text	.sect

;*******************************************************
;*
;* __start - C runtime initialization
;*
;*  Clears BSS, sets up __base and __break, parses
;*  command line, calls __main.
;*
;*******************************************************

m.init:
__start:
	ldk	r2, #0
	ldk	r3, #0

	ld	r2, ARG1(r15)	; r2 -> basepage
	ld	r4, (lbss+2)(r2)  ; r4 -> bss start (offset only)
	ld	r1, (bsslen+2)(r2) ; r1 = bss length
	lda	r6, r1(r4)	; r6 -> end of bss
	rr	r1, #1		; word count
	jr	z, xdone

xclear:
	clr	@r4
	inc	r4, #2
	djnz	r1, xclear

xdone:
	ld	__base, r2	; Save basepage pointer
	ld	__break, r6	; Set up break address

	lda	r4, (command+1)(r2) ; r4 -> command line text
	ldb	rl2, -1(r4)	; r2 = command line length
	ldb	rh2, #0
	push	@r15, r2	; Push length
	push	@r15, r4	; Push command line address
	ldk	r14, #0		; Clear frame pointer

	call	_main		; Call C main

__exit:
	ldk	r5, #EXIT	; Warm boot
	sc	#BDOS_SC

;*******************************************************
;*
;* _brk - change break address
;*
;*  int brk(char *addr)
;*  Returns 0 on success, -1 if too close to stack.
;*  Alcyon convention: return in R0.
;*
;*******************************************************

_brk:
	ld	r0, ARG1(r15)	; New break address
	ld	r2, r0
	add	r2, #safety	; Add safety margin
	cp	r2, r15		; Compare with stack
	jr	ult, brkok
	ld	r0, #0
	dec	r0, #1		; Return -1
	ret

brkok:
	ld	__break, r0	; Save new break
	ldk	r0, #0		; Return 0
	ret

;*******************************************************
;*
;* ___BDOS - C interface to BDOS system calls
;*
;*  int __BDOS(int func, int param)
;*
;*  BDOS expects: r5=function, rr6=parameter
;*  BDOS returns: r7=result
;*  Alcyon convention: return in R0.
;*
;*  On Z8002 (non-segmented), param is 16-bit and goes
;*  in r7 (low word of rr6).  r6 is cleared.
;*
;*******************************************************

___BDOS:
	ld	r5, ARG1(r15)	; function number
	ldk	r6, #0		; high word = 0 (non-segmented)
	ld	r7, ARG2(r15)	; parameter (16-bit)
	sc	#BDOS_SC	; Enter BDOS

	cp	r15, __break	; Check for stack overflow
	jr	ult, __sovf

	ld	r0, r7		; Copy BDOS return to R0 (Alcyon convention)
	ret

__sovf:
	ldk	r5, #PRINT	; Print error message
	ldk	r6, #0
	ld	r7, ovf
	sc	#BDOS_SC
	jr	__exit

	.end
