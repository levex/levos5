#ifndef __MM_H_
#define __MM_H_

#include <stddef.h>
#include <stdint.h>

struct	boundary_tag
{
	unsigned int magic;			//< It's a kind of ...
	unsigned int size; 			//< Requested size.
	unsigned int real_size;		//< Actual size.
	int index;					//< Location in the page table.

	struct boundary_tag *split_left;	//< Linked-list info for broken pages.	
	struct boundary_tag *split_right;	//< The same.
	
	struct boundary_tag *next;	//< Linked list info.
	struct boundary_tag *prev;	//< Linked list info.
};


 
extern int liballoc_lock();
extern int liballoc_unlock();
extern void* liballoc_alloc(int);
extern int liballoc_free(void*,int);

       

void     *malloc(size_t);
void     *realloc(void *, size_t);
void     *calloc(size_t, size_t);
void      free(void *);	

void* memset (void * ptr, int value, int num );
void memcpy(uint8_t *dest, uint8_t *src, uint32_t len);

#endif
