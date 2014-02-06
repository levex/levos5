#include <display.h>
#include <device.h>
#include <mm.h>

struct device **device_list;
static int device_count = 0;

int device_core_init()
{
	device_list = malloc(MAX_DEVICES * sizeof(uint32_t));
	if (!device_list)
		return 1;
	
	memset(device_list, 0, MAX_DEVICES * sizeof(uint32_t));
	return 0;
}

int device_register(struct device *dev)
{
	if (!dev || !dev->valid)
		return 1;
	
	for (int i = 0; i < MAX_DEVICES; i++) {
		if (! device_list[i]) {
			device_list[i] = dev;
			device_count ++;
			printk("Registered device %s\n", dev->name);
			break;
		}
	}
	return 0;
}

struct device *get_device(int i)
{
	return device_list[i];
}
int device_get_count()
{
	return device_count;
}
