/** @author Levente Kurusa <levex@linux.com> **/
#include <stdint.h>
#include <elf.h>
#include <scheduler.h>
#include <string.h>
#include <mm.h>
#include <hal.h>

static uint32_t started = 0;

elf_priv_data *elf_probe(uint8_t *buffer)
{
	elf_header_t *header = (elf_header_t *)buffer;
	/* The first four bytes are 0x7f and 'ELF' */
	if(header->e_ident[0] == 0x7f && 
		header->e_ident[1] == 'E' && header->e_ident[2] == 'L' && header->e_ident[3] == 'F')
	{
		/* Valid ELF! */
		return (void *)1;
	}
	return 0;
}

/*static uint32_t __addr = 0x800000;
uint32_t phys_alloc(uint32_t size)
{
	uint32_t ret = __addr;
	__addr = (__addr + (size * 4)) & 0xfffff000;
	return ret;
}

uint32_t get_palloc_len(uint32_t palloc)
{
	return __addr - palloc;
}*/

uint8_t elf_start(uint8_t *buf, uint32_t esize, char *const argv[], char *envp[], elf_priv_data *priv)
{
	envp = envp;
	priv = priv;
	elf_header_t *header = (elf_header_t *)buf;
	if(header->e_type != 2)
	{
		return 0;
	}
	/* Find first program header and loop through them */
	elf_program_header_t *ph = (elf_program_header_t *)(buf + header->e_phoff);
	/* create new page directory */
	uint32_t palloc = (uint32_t)phymem_alloc(esize);
	uint32_t opalloc = get_process()->palloc;
	uint32_t opalloc_len = get_process()->palloc_len;
	uint32_t paged;
	arch_setup_paged(&paged, palloc);
	started ++;
	//printk("Would set to 0x%x\n", (uint32_t)paged); for(;;);
	get_process()->paged = paged;
	get_process()->palloc = palloc;
	get_process()->palloc_len = esize;
	get_process()->exec_len = esize;

	/* schedule away to update cr3! */
	schedule_noirq();
	for(int i = 0; i < header->e_phnum; i++, ph++)
	{
		switch(ph->p_type)
		 {
		 	case 0: /* NULL */
		 		break;
		 	case 1: /* LOAD */
		 		/*printk("LOAD: offset 0x%x vaddr 0x%x paddr 0x%x filesz 0x%x memsz 0x%x\n",
		 				ph->p_offset, ph->p_vaddr, ph->p_paddr, ph->p_filesz, ph->p_memsz);*/
		 		memcpy((uint8_t *)ph->p_vaddr, buf + ph->p_offset, ph->p_filesz);
		 		break;
		 	default: /* @TODO add more */
		 	 return 0;
		 }
	}

	/* Program loaded, jump to execution */
	int argc = 0;
	if(argv)
		while(argv[argc]) argc++;

	free(buf);
	if(opalloc)
		phymem_free((void *)opalloc, opalloc_len);
	START_EXECUTION_BY_JUMPING(header->e_entry, argv, argc);
	return 0;
}
