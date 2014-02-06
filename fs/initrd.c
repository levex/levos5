#include <vfs.h>
#include <stdint.h>
#include <device.h>

#define IDPRIV(x) ((struct initrd_priv *) x->priv )

int initrd_read(struct device *dev, uint8_t *buf, uint32_t st, uint32_t len)
{
	if(IDPRIV(dev)->start + len * 512 > IDPRIV(dev)->end) return 0;
	//printk("%s: dev=0x%x, buf=0x%x, st=%d, len=%d\n", __func__,
		//dev, buf, st, len);
	memset(buf, 0, len*512);
	memcpy(buf, IDPRIV(dev)->start + st * 512, len * 512);
	//printk("copied...\n");
	return 0;
}

int initrd_write(struct device *dev, uint8_t *buf, uint32_t st, uint32_t len)
{
	return 0;
}

struct device initrd_device = {
	.valid = 1,
	.id = 13124,
	.flags = DEVICE_FLAG_BLOCK | DEVICE_FLAG_NOWRITE,
	.name = "initrd",
	.fs = 0,
	.read = initrd_read,
	.write = initrd_write,
	.priv = 0,
};

int initrd_create(uint32_t *s, uint32_t *e)
{
	initrd_device.priv = malloc(sizeof(struct initrd_priv));
	((struct initrd_priv *) initrd_device.priv )->start = s;
	((struct initrd_priv *) initrd_device.priv )->end = e;
	printk("Initrd going from 0x%x to 0x%x\n", s, e);
	device_register(&initrd_device);
}
