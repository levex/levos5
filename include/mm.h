#ifndef __MM_H_
#define __MM_H_

#include <stddef.h>
#include <stdint.h>
#include <mutex.h>
#include <hal.h>

#ifndef _LIBALLOC_H
#define _LIBALLOC_H

/** \defgroup ALLOCHOOKS liballoc hooks 
 *
 * These are the OS specific functions which need to 
 * be implemented on any platform that the library
 * is expected to work on.
 */

/** @{ */



// If we are told to not define our own size_t, then we skip the define.
//#define _HAVE_UINTPTR_T
//typedef	unsigned long	uintptr_t;

//This lets you prefix malloc and friends
#define PREFIX(func) func

#ifdef __cplusplus
extern "C" {
#endif


       

extern void    *PREFIX(malloc)(size_t);				///< The standard function.
extern void    *PREFIX(realloc)(void *, size_t);		///< The standard function.
extern void    *PREFIX(calloc)(size_t, size_t);		///< The standard function.
extern void     PREFIX(free)(void *);					///< The standard function.


#ifdef __cplusplus
}
#endif


/** @} */

#endif




void* memset (void * ptr, int value, int num );
void memcpy(uint8_t *dest, uint8_t *src, uint32_t len);
void* memset16 (void * ptr, uint16_t value, int num );

extern void phymem_init_level_one(uint32_t start);
extern void phymem_init_level_two(uint32_t size);
extern void *phymem_alloc(uint32_t size);
extern void phymem_free(void *ptr, uint32_t size);

#endif
