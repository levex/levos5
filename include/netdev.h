#ifndef __NETDEV_H_
#define __NETDEV_H_

#include <stdint.h>
#include <pci.h>

struct packet {
	uint32_t len;
	uint8_t *payload;
};

typedef uint8_t mac_t[6];

struct net_device {

	mac_t macaddr;
	struct pci_device *pdev;

};

#endif
