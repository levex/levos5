#ifndef __DUMMYFS_H_
#define __DUMMYFS_H_

#include <device.h>

int dummyfs_probe(struct device *dev);
int dummyfs_mount(struct device *dev);

extern void dummyfs_init();

extern struct device dummyfs_dev;

#endif
