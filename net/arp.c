#include <netdev.h>
#include <mm.h>
#include <arp.h>

uint16_t __swap16(uint16_t a)
{
	uint16_t b0,b1;
	b0 = (a & 0x00ff) << 8;
	b1 = (a & 0xff00) >> 8;
	
	return b0|b1;
}

void arp_get_ip(struct net_device *ndev)
{
	printk("Using ARP to find IP address!\n");
	struct arp_packet *arp = malloc(60);
	arp->hlen = 6;
	arp->plen = 4;
	arp->htype = __swap16(0x01);
	arp->ptype = __swap16(0x0800);

	arp->opcode = __swap16(0x0001);

	uint8_t mac[6];
	net_query_mac(mac);
	arp->srchw[0] = mac[5];
	arp->srchw[1] = mac[4];
	arp->srchw[2] = mac[3];
	arp->srchw[3] = mac[2];
	arp->srchw[4] = mac[1];
	arp->srchw[5] = mac[0];
	memset(arp->srcpr, 0, arp->plen);
	memset(arp->dstpr, 0, arp->plen);
	memset(arp->dsthw, 0, arp->hlen);

	struct packet pk = {
		.len = 60,
		.payload = arp,
	};

	ndev->send_packet(ndev, &pk);

	free(arp);
}
