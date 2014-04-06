#ifndef __PCI_H_
#define __PCI_H_

#include <stdint.h>

struct pci_device;

struct pci_driver {
	int (*probe)(struct pci_device *pdev);

};

struct pci_device {
	uint16_t vendor;
	uint16_t device;
	
	uint32_t bus;
	uint32_t slot;
	uint16_t func;

	uint32_t membase;
	uint32_t memsize;
	uint32_t iobase;
	uint32_t irq;

	void *data;
	struct pci_driver *drv;
};

void pci_init();

void pci_register_driver(struct pci_driver *pdrv);

#endif
