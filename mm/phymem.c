#include <mm.h>
#include <stdint.h>
#include <bitmap.h>

static uint32_t __phy_start = 0;
static uint32_t __phy_size = 0;
static uint32_t __phy_end = 0;
static uint8_t __phy_state = 0;

#define PHY_STATE_INIT_ZER 0
#define PHY_STATE_INIT_ONE 1
#define PHY_STATE_INIT_TWO 2

uint8_t *phy_bitmap = 0;

/**
 * phymem_init_level_one - Set the allocation minimum
 * 
 * THIS IS LEVEL ONE OF INIT, LEVEL TWO WILL BE CALLED IN main()
 */
void phymem_init_level_one(uint32_t start)
{
	/* initialize __phy_start */
	__phy_start = start;
	__phy_state = PHY_STATE_INIT_ONE;
}

/**
 * phymem_init_level_two - Set end of memory
 * 
 * THIS IS LEVEL __TWO__ of INIT.
 */
void phymem_init_level_two(uint32_t size)
{
	if (__phy_state != PHY_STATE_INIT_ONE)
		panic("phymem state is not one in level2!\n");

	/* setup mandatory variables */
	__phy_size = size;
	__phy_end = __phy_start + __phy_size;
	
	/* setup bitmap */
	
	/* find out the amount of pages */
	int pages = (size / 4096) + 1;
	/* find out the number of bytes required */
	int bytes = (pages / 8) + 1;
	/* allocate the bitmap */
	phy_bitmap = malloc(bytes);
	/* zero out the bitmap */
	memset(phy_bitmap, 0, bytes);
	/* we are ready */
	__phy_state = PHY_STATE_INIT_TWO;
}

/**
 * phymem_alloc_pages - Allocate a set of pages in the phy space
 * 
 * \returns a pointer to the start of the allocation
 */
void *phymem_alloc_pages(uint32_t pages)
{
	if (__phy_state != PHY_STATE_INIT_TWO)
		panic("Physical memory allocation is not ready\n");
	
	int start = bitmap_find_zero(phy_bitmap, pages);
		
	return (void *) (__phy_start + start * 0x1000);
}

/**
 * phymem_free_pages - Free a set of pages from start
 */
void phymem_free_pages(void *start, uint32_t pages)
{
	if (__phy_state != PHY_STATE_INIT_TWO)
		panic("Physical memory freeing is not ready\n");
		
	/* find starting page */
	int ps = ((uint32_t)start - __phy_start) / 0x1000;
	for(int i = 0; i < pages; i++)
		unset_bitmap(phy_bitmap, ps + i);
}

/**
 * phymem_alloc - Allocate physpace as size
 */
void *phymem_alloc(uint32_t size)
{
	/* find out how many pages do we need */
	int ps = (size / 0x1000) + 1;
	return phymem_alloc_pages(ps);
}

/**
 * phymem_free - Free physpace from ptr
 */
void phymem_free(void *ptr, uint32_t size)
{
	/* find out how many pages do we need */
	int ps = (size / 0x1000) + 1;
	phymem_free_pages(ptr, ps);
}
