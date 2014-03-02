interrupt_vector_table:
    b . @ Reset
    b . 
    b . @ SWI instruction
    b . 
    b .
    b .
    b .
    b .

.comm stack, 0x10000 @ Reserve 64k stack in the BSS
_start:
    .globl _start
    ldr sp, =stack+0x10000 @ Set up the stack
    bl main @ Jump to the main function
    mov	r8, #0x78
	add	r8, r8, #0x56 << 8
	add	r8, r8, #0x34 << 16
	add	r8, r8, #0x12 << 24
1: 
    b 1b @ Halt

.globl __arm__put32
__arm__put32:
	str r1, [r0]
	bx lr

.globl __arm__get32
__arm__get32:
	ldr r0, [r0]
	bx lr

.globl dummy
dummy:
	bx lr
