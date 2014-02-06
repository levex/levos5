#ifndef __PROCFS_H_
#define __PROCFS_H_

#include <device.h>

int procfs_probe(struct device *dev);
int procfs_mount(struct device *dev);

extern struct device procfs_dev;

#endif
