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

int paging_init()
{
	uint32_t page_aligned_end = ((uint32_t)&kernel_end & 0xfffff000) + 0x1000;
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
	uint32_t *__first_pt = (uint32_t *) page_aligned_end;
	page_aligned_end += 0x1000;
	uint32_t *__second_pt = (uint32_t *) page_aligned_end;
	/* now, lets map them */
	for(int i = 0; i < 1024; i++)
	{
		__kernel_heap_pt[i] = make_page_pte(page_aligned_end + i * 0x1000, KERNEL_PAGE);
		__first_pt[i] =       make_page_pte(0 + i * 0x1000, KERNEL_PAGE);
		__second_pt[i] = 	  make_page_pte(0x400000 + i * 0x1000, KERNEL_PAGE);
	}
	/* add them to the page directory */
	page_directory[768] = (uint32_t)__kernel_heap_pt | 3;
	page_directory[0] = (uint32_t)__first_pt | 3;
	page_directory[1] = (uint32_t) __second_pt | 3;
	for(int i = 0; i < 128; i++)
		kheap_bitmap[i] = 0;
	/* enable paging */
	asm volatile("mov %0, %%cr3"::"b"(page_directory));
	uint32_t cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0));
	return 0;
}

/* try to allocate #pages of continus pages */
void *mm_alloc_pages(int pages)
{
	return (void *)(KHEAP_VIRTUAL_START + bitmap_find_zero(kheap_bitmap, pages) * 0x1000);
}

int mm_free_pages(void *ptr, int pages)
{
	/* determine bit id from the ptr */
	int id = ((uint32_t)ptr - KHEAP_VIRTUAL_START) / 0x1000;
	for(int i = 0; i < pages; i++)
		unset_bitmap(kheap_bitmap, id + i);
	return 0;
}

