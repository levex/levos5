#include <pci.h>
#include <netdev.h>
#include <display.h>
#include <mm.h>

struct net_device *ndevs[8];
uint32_t count = 0;
uint32_t netmain = 0;

void register_net_device(struct net_device *ndev)
{
	ndevs[count] = ndev;
	count ++;
	printk("net: registered new network device\n");
	netmain = count - 1;
}

void net_query_mac(uint8_t *mac)
{
	memcpy(mac, ndevs[netmain]->macaddr, 6);
}
