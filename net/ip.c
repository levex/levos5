#include <netdev.h>
#include <pci.h>

void ip_find(struct net_device *ndev)
{
	arp_get_ip(ndev);
}
