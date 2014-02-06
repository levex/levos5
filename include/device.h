#ifndef __DEVICE__H_
#define __DEVICE__H_

#define MAX_DEVICES 128

#include <stdint.h>

//struct device;

#define DEVICE_FLAG_BLOCK 1
#define DEVICE_FLAG_NOWRITE 2
#define DEVICE_FLAG_NOREAD 4
#define DEVICE_FLAG_BROKEN 8

struct device {
	uint8_t valid;
	uint32_t id;
	uint8_t flags;
	char *name;
	
	struct filesystem *fs;
	
	int (*read)(struct device *dev, uint8_t *buf, uint32_t st, uint32_t len);
	int (*write)(struct device *dev, uint8_t *buf, uint32_t st, uint32_t len);
	
	void *priv;
};

struct device *get_device(int i);
int device_get_count();
int device_core_init();
int device_register(struct device *dev);

#endif
