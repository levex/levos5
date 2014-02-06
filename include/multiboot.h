#ifndef __MULTIBOOT_H_
#define __MULTIBOOT_H_

#include <stdint.h>

struct multiboot {
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t mods_addr;
	/* etc, we dont need 'em */
};

struct multiboot_mod {
	uint32_t mod_start;
	uint32_t mod_end;
	
	uint32_t cmdline;
	
	uint32_t pad;
};

#endif
