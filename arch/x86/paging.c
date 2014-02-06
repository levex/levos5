/* @author Levente Kurusa <levex@linux.com> */

#include <mm.h>
#include <stdint.h>
#include <bitmap.h>

extern int kernel_base;
extern int kernel_end;

uint8_t kheap_bitmap[128];

static uint32_t *page_directory = 0;
static uint32_t *__kernel_heap_pt = 0;

#define KHEAP_VIRTUAL_START 0xC0000000
#define KERNEL_PAGE 3

uint32_t make_page_pte(uint32_t addr_aligned, int flags)
{
	return addr_aligned | flags;
}

void initialize_pt(uint32_t *pt, uint32_t startaddr)
{
	for(int i = 0;i < 1024; i++) {
		pt[i] = make_page_pte(startaddr + i * 0x1000, KERNEL_PAGE);
	}
}

uint32_t *__first_pt = 0;

int paging_init()
{
	uint32_t page_aligned_end = ((uint32_t)0x300000 & 0xfffff000) + 0x1000;
	/* paging data will be immediately after the kernel,
	 * we null out 4kb of space and call malloc_init()
	 * to initialize the allocator
	 */
	page_directory = (uint32_t *)page_aligned_end;
	for(int i = 0; i < 1024; i++)
	{
		/* set each entry to kernel, rw, not present */
		page_directory[i] = 0 | 2;
	}
	/* we need to reserve extra space for a page table,
	 * this page table will map starting of the kernel heap
	 * which will be from 0xC0000000 (pd[768])
 	 */
	page_aligned_end += 0x1000;
	__kernel_heap_pt = (uint32_t *) page_aligned_end;
	/* also reserve space to map the first 4 megabytes */
	page_aligned_end += 0x1000;
	__first_pt = (uint32_t *) page_aligned_end;
	/* now, lets map them */
	initialize_pt(__first_pt, 0);
	initialize_pt(__kernel_heap_pt, 0x400000);
	/* add them to the page directory */
	page_directory[768] = (uint32_t)__kernel_heap_pt | 3;
	page_directory[0] = (uint32_t)__first_pt | 3;
	//page_directory[1] = (uint32_t) __second_pt | 3;
	//page_directory[256] = (uint32_t) __third_pt | 3;
	for(int i = 0; i < 128; i++)
		kheap_bitmap[i] = 0;
	/* Initialize physical memory allocator,
	 * yes this must be initialized here,
	 * heap is handled here, but other parts of the kernel
	 * require allocation of physical memory and hence
	 * we initialize it here. BEFORE PAGING!
	 */
	phymem_init_level_one(page_aligned_end + 0x1000);
	/* enable paging */
	asm volatile("mov %0, %%cr3"::"b"(page_directory));
	uint32_t cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0));
	return 0;
}



uint32_t *create_new_page_directory(uint32_t vaddr, uint32_t paddr) {
	/* so, we gotta create a new page directory, so alloc space for it */
	uint32_t *pd = malloc(8192);
	while((uint32_t)pd & ~0xfffff000)
		pd = (uint32_t*)((uint32_t)pd + 1);
	/* map the kernel to it, we can use the same tables here */
	pd[768] = (uint32_t) __kernel_heap_pt | 3;
	//printk("pd[768]= 0x%x\n", pd[768]);
	pd[0] = (uint32_t) __first_pt | 3;
	//printk("pd[0]= 0x%x\n", pd[0]);
	/* kernel mapped, let's reserve space for a new page table */
	uint32_t *pt = malloc(8192);
	while((uint32_t)pt & ~0xfffff000)
		pt = (uint32_t*)((uint32_t)pt + 1);
	/* initialize it */
	initialize_pt(pt, paddr);
	/* add to pagedirectory */
	pd[vaddr >> 22] = (uint32_t ) pt | 3;
	/* convert it to heap paddr */
	pd[vaddr >> 22] -= 0xC0000000;
	pd[vaddr >> 22] += 0x00400000;
	//printk("pd[%d] = 0x%x\n", vaddr >> 22, pd[vaddr >> 22]);
	/* now convert pd to heap paddr */
	pd = (uint32_t *)((uint32_t)pd - 0xC0000000 + 0x400000);
	return pd;
}

/* replace a page in a pagetable to page */
void replace_page(uint32_t **pd, uint32_t vaddr, uint32_t page) {
	uint32_t vid = vaddr >> 22;
	uint32_t pageid = vaddr % 4194304;
	
	pd[vid][pageid] = page;
	return;
}

/* try to allocate #pages of continus pages */
void *mm_alloc_pages(int pages)
{
	return (void *)(KHEAP_VIRTUAL_START + bitmap_find_zero(kheap_bitmap, pages) * 0x1000);
}

int mm_free_pages(void *ptr, int pages)
{
	return 0;
	/* determine bit id from the ptr */
	int id = ((uint32_t)ptr - KHEAP_VIRTUAL_START) / 0x1000;
	for(int i = 0; i < pages; i++)
		unset_bitmap(kheap_bitmap, id + i);
	return 0;
}

