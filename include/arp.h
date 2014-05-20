#ifndef __ARP_H
#define __ARP_H

#include <stdint.h>

#define ARP_HTYPE_ETHERNET 0x01
#define ARP_PTYPE_IP	   0x0800

struct arp_packet {
	uint16_t htype;
	uint16_t ptype;
	uint8_t hlen;
	uint8_t plen;
	uint16_t opcode;
	uint8_t srchw[6];
	uint8_t srcpr[4];
	uint8_t dsthw[6];
	uint8_t dstpr[4];
};

#endif
