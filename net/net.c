#include <pci.h>
#include <netdev.h>
#include <display.h>
#include <mm.h>

struct net_device *ndevs[8];
uint32_t count = 0;
uint32_t netmain = 0;

uint8_t macaddr[6];

void register_net_device(struct net_device *ndev)
{
	ndevs[count] = ndev;
	count ++;
	printk("net: registered new network device\n");
}

void net_set_mac(uint8_t *m)
{
	memcpy(macaddr, m, 6);
}

struct net_device *net_getmain()
{
	if (count == 0)
		return 0;
	
	return ndevs[0];
}

void net_query_mac(uint8_t *mac)
{
	memcpy(mac, &macaddr[0], 6);
}
