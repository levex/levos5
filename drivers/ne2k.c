#include <hal.h>
#include <ne2k.h>
#include <display.h>
#include <pci.h>
#include <netdev.h>

struct ne2k_device {
	uint8_t nextmempage;
	struct pci_device *pdev;
	struct net_device *ndev;
};

int ne2k_setup(struct pci_device *pdev)
{
	/* set page 0, abort remote DMA, and STOP */
	outportb(pdev->iobase + NE_P0_CR, NE_CR_RD2 | NE_CR_STP);

	/* set FIFO threshold to 8, no auto-init rDMA, byteorder 8086, wwDMA */
	outportb(pdev->iobase + NE_P0_DCR, NE_DCR_FT1 | NE_DCR_WTS | NE_DCR_LS);

	return 0;
}

extern void ne2k_irq();
void __imp_ne2k_irq()
{
	printk("Ne2k: IRQ!\n");
	send_eoi(11);
}

int ne2k_setup_2(struct pci_device *pdev, uint8_t mac[6])
{
	if(!pdev || !mac)
		return 1;
	
	/* p0, abort rDMA, stop */
	outportb(pdev->iobase + NE_P0_CR, NE_CR_RD2 | NE_CR_STP);

	/* setup receive ring buf */
	outportb(pdev->iobase + NE_P0_PSTART, 0x40);
	outportb(pdev->iobase + NE_P0_PSTOP, 0xC0);
	outportb(pdev->iobase + NE_P0_BNRY, 0xBF);

	/* enable some interrupts */
	outportb(pdev->iobase + NE_P0_IMR, NE_IMR_PRXE | NE_IMR_PTXE |
			NE_IMR_RXEE | NE_IMR_TXEE | NE_IMR_OVWE | NE_IMR_RDCE);

	/* p1, stop rDMA, stop */
	outportb(pdev->iobase + NE_P0_CR, NE_CR_PAGE_1 | NE_CR_RD2 | NE_CR_STP);

	/* setup MAC address */
	for(int i = 0;i < 6; i++)
			outportb(pdev->iobase + NE_P1_PAR0 + i, mac[i]);

	/* set current page */
	outportb(pdev->iobase + NE_P1_CURR, 0x41);

	/* setup multicast */
	for(int i = 0; i < 8; i++)
			outportb(pdev->iobase + NE_P1_MAR0 + i, 0);
	
	/* p0, rDMA stop, stop */
	outportb(pdev->iobase + NE_P0_CR, NE_CR_RD2 | NE_CR_STP);

	/* accept broadcast */
	outportb(pdev->iobase + NE_P0_RCR, NE_RCR_AB);

	/* no loopback */
	outportb(pdev->iobase + NE_P0_TCR, 0);

	/* no interrupts */
	outportb(pdev->iobase + NE_P0_ISR, 0xFF);

	/* register interrupt handler */
	register_interrupt(pdev->irq + 32, ne2k_irq);	

	/* start! */
	outportb(pdev->iobase + NE_P0_CR, NE_CR_RD2 | NE_CR_STA);

	printk("ne2k: setup complete!\n");


	return 0;
}

void ne2k_readmem(struct pci_device *pdev, uint16_t *buf, uint32_t offset, uint32_t len) {

	if (len & 1) len ++;

	/* abort rDMA */
	outportb(pdev->iobase + NE_P0_CR, NE_CR_RD2 | NE_CR_STA);

	/* setup DMA bytes */
	outportb(pdev->iobase + NE_P0_RBCR0, (uint8_t)len);
	outportb(pdev->iobase + NE_P0_RBCR1, (uint8_t)(len >> 8));

	/* setup NIC mem source */
	outportb(pdev->iobase + NE_P0_RSAR0, (uint8_t) offset);
	outportb(pdev->iobase + NE_P0_RSAR1, (uint8_t) (offset >> 8));

	/* read! */
	outportb(pdev->iobase + NE_P0_CR, NE_CR_RD0 | NE_CR_STA);

	for(int i = 0; i < (len >> 1); i++) {
		buf[i] = inportw(pdev->iobase + 0x10 + NE_NOVELL_DATA);
	}

}

uint8_t ne2k_get_write_page(struct pci_device *pdev, uint16_t len)
{
	struct ne2k_device *dev = pdev->data;

	uint8_t ret = dev->nextmempage;

	dev->nextmempage += (len + 0xFF) >> 8;
	if (dev->nextmempage >= 0xC0) {
		dev->nextmempage -= 0x40;
	}
}

int ne2k_send_packet(struct net_device *ndev, struct packet *p)
{
	struct pci_device *pdev = ndev->pdev;
	uint32_t io = pdev->iobase;

	/* p0, noDMA start */
	outportb(io + NE_P0_CR, NE_CR_RD2 | NE_CR_STA);

	/* reset rDMA flag */
	outportb(io + NE_P0_ISR, NE_ISR_RDC);

	/* setup bytes */
	outportb(io + NE_P0_TBCR0, p->len & 0xFF);
	outportb(io + NE_P0_TBCR1, p->len >> 8);

	/* setup remote bytes */
	outportb(io + NE_P0_RBCR0, p->len & 0xFF);
	outportb(io + NE_P0_RBCR1, p->len >> 8);

	/* setup transfer */
	outportb(io + NE_P0_RSAR0, 0x00);

	int page = ne2k_get_write_page(pdev, p->len);

	outportb(io + NE_P0_RSAR1, page);

	/* p0, remote write, start */
	outportb(io + NE_P0_CR, NE_CR_RD1 | NE_CR_STA);

	/* copy packet buffer */
	int buflen = p->len;
	uint16_t *buf = p->payload;
	while(buflen --)
		outportw(io + 0x10, *buf++);

	/* wait for complete */
	while (!inportb(io + NE_P0_ISR) & 0x40)
		schedule_noirq();

	/* ack IRQ */
	outportb(io + NE_P0_ISR, 0x40);

	/* now send it */
	outportb(io + NE_P0_TPSR, page);
	outportb(io + NE_P0_CR, 0|0x10|0x4|0x2);

	return 0;

}

int ne2k_probe(struct pci_device *pdev)
{
	if (pdev->vendor != 0x10EC || pdev->device != 0x8029)
		return 1;

	uint32_t base = pdev->iobase;
	printk("NE2000: found PCI device! iobase=0x%x\n", base);
	ne2k_setup(pdev);
	
	uint16_t w[16];
	uint8_t mac[6];
	ne2k_readmem(pdev, w, 0, 16);
	for(int i = 0; i < 6; i++)
		mac[i] = w[i];
	printk("MAC: %x.%x.%x.%x.%x.%x\n", mac[0], mac[1], mac[2], mac[3],
			mac[4], mac[5]);

	ne2k_setup_2(pdev, mac);
	
	struct ne2k_device *ne = malloc(sizeof(*ne));
	ne->pdev = pdev;
	pdev->data = ne;

	struct net_device *ndev = malloc(sizeof(*ndev));
	ndev->pdev = pdev;
	ndev->send_packet = ne2k_send_packet;
	memcpy(ndev->macaddr, mac, 6);

	register_net_device(ndev);

	struct packet *pk = malloc(sizeof(*pk));
	void *buf = malloc(14);
	memcpy(buf, "Hello, world!\n", 14);
	pk->payload = buf;
	pk->len = 14;
	ne2k_send_packet(ndev, pk);

	return 0;
}

struct pci_driver ne2k_drv = {
	.probe = ne2k_probe,
};

int ne2k_init()
{
	printk("Loading Ne2000 driver\n");
	pci_register_driver(&ne2k_drv);
	return 0;
}
