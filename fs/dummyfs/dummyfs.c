#include <hal.h>
#include <vfs.h>
#include <dummyfs.h>
#include <device.h>
#include <mm.h>

struct device dummyfs_dev = {
	.valid = 1,
	.id = 1337,
	.flags = DEVICE_FLAG_BLOCK,
	.name = "dummydev",
	
	.read = 0,
	.write = 0,
};

struct filesystem dummyfs_default = {
	.name = "dummyfs",
	.probe = dummyfs_probe,
	.readfull = 0,//uint8_t (*read)(char *, char *, struct device *, void *);
	.read_dir = 0, //uint8_t (*read_dir)(char *, char *, struct device *, void *);
	.touch = 0,//uint8_t (*touch)(char *fn, struct device *, void *);
	.writefile = 0,//uint8_t (*writefile)(char *fn, char *buf, uint32_t len, struct device *, void *);
	.exist = 0,//uint8_t (*exist)(char *filename, struct device *, void *);
	.mount = dummyfs_mount,//uint8_t (*mount)(struct device *, void *);
};

int dummyfs_probe(struct device *dev)
{
	//if (!dev->valid)
		return 1;
		
	/* allocate space for a new filesystem */
	struct filesystem *fs = malloc(sizeof(struct filesystem));
	/* copy the default to it */
	memcpy((uint8_t *)fs, (uint8_t *)&dummyfs_default, sizeof(struct filesystem));	
	/* set the device's filesystem to it */
	dev->fs = fs;
	
	printk("Probing for dummyfs\n");
	
	/* probe successful */
	return 0;
}

int dummyfs_mount(struct device *dev)
{
	if (!dev)
		return 1;
	
	printk("Mount dummyfs successful!\n");	
	
	return 0;
}

void dummyfs_init()
{
	device_register(&dummyfs_dev);
}
