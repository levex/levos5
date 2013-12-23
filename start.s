.set ALIGN,    1<<0             
.set MEMINFO,  1<<1            
.set FLAGS,    ALIGN | MEMINFO  
.set MAGIC,    0x1BADB002       
.set CHECKSUM, -(MAGIC + FLAGS)


.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bootstrap_stack
stack_bottom:
.skip 32768 # 16 KiB
stack_top:

.section .text

.global __idt_default_handler
.type __idt_default_handler, @function
__idt_default_handler:
	pushal
	mov $0x20, %al
	mov $0x20, %dx
	out %al, %dx
	#call _test
	popal
	iretl

.global _set_gdtr
.type _set_gdtr, @function
_set_gdtr:
	push %ebp
	movl %esp, %ebp

	lgdt 0x800

	movl %ebp, %esp
	pop %ebp
	ret

.global _set_idtr
.type _set_idtr, @function
_set_idtr:
	push %ebp
	movl %esp, %ebp

	lidt 0x10F0

	movl %ebp, %esp
	pop %ebp
	ret

.global _reload_segments
.type _reload_segments, @function
_reload_segments:
	push %ebp
	movl %esp, %ebp

	push %eax
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	pop %eax

	ljmp $0x8, $me
me:
	movl %ebp, %esp
	pop %ebp
	ret

.global _start
.type _start, @function
_start:

	movl $stack_top, %esp
	mov $0x1337, %eax

	call main
	
	mov $0xdeadc0de, %eax

	cli
	hlt

.Lhang:
	jmp .Lhang


.size _start, . - _start

