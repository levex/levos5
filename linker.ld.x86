ENTRY(_start)
SECTIONS
{
	. = 0x100000;
	kernel_base = .;
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
    .bootstrap_stack BLOCK(4K) : ALIGN(4K)
    {
        *(.bootstrap_stack)
    }
	kernel_end = .;
}
