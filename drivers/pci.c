#include <stdint.h>
#include <display.h>
#include <pci.h>

struct pci_device *pdevs = 0;
uint8_t pci_devices = 0;

struct pci_driver **pdrvs = 0;
uint8_t pci_drivers = 0;

void pci_register_driver(struct pci_driver *pdrv)
{
	pdrvs[pci_drivers] = pdrv;
	pci_drivers ++;

	for (int i = 0; i < pci_devices; i++)
	{
		pdrv->probe(&pdevs[i]);
	}
}

uint8_t pci_read_byte(uint8_t bus, uint8_t slot,
						uint8_t func, uint32_t offset)
{
	uint32_t id = ((bus) << 16) | ((slot) << 11) | ((func) << 8);
	uint32_t addr = 0x80000000 | id | (offset & 0xfc);
	outportl(0xCF8, addr);
	return inportb(0xCFC + (offset & 0x03));
}

void pci_write_byte(uint8_t bus, uint8_t slot,
						uint8_t func, uint32_t offset, uint8_t byte)
{
	uint32_t id = ((bus) << 16) | ((slot) << 11) | ((func) << 8);
	uint32_t address = 0x80000000 | id | (offset & 0xfc);
	outportl(0xCF8, address);
	outportb(0xCFC + (offset & 0x03), byte);
}

uint16_t pci_read_word(uint8_t bus, uint8_t slot,
						uint8_t func, uint32_t offset)
{
	uint32_t id = ((bus) << 16) | ((slot) << 11) | ((func) << 8);
	uint32_t addr = 0x80000000 | id | (offset & 0xfc);
	outportl(0xCF8, addr);
	return inportw(0xCFC + (offset & 0x02));

}

void pci_write_word(uint8_t bus, uint8_t slot,
						uint8_t func, uint32_t offset, uint16_t word)
{
	uint32_t id = ((bus) << 16) | ((slot) << 11) | ((func) << 8);
	uint32_t addr = 0x80000000 | id | (offset & 0xfc);
	outportl(0xCF8, addr);
	outportw(0xCFC + (offset & 0x02), word);
}

uint32_t pci_read_dword(uint8_t bus, uint8_t slot,
						uint8_t func, uint32_t offset)
{
	uint32_t id = ((bus) << 16) | ((slot) << 11) | ((func) << 8);
	uint32_t addr = 0x80000000 | id | (offset & 0xfc);
	outportl(0xCF8, addr);
	return inportl(0xCFC);
}

void pci_write_dword(uint8_t bus, uint8_t slot,
						uint8_t func, uint32_t offset, uint32_t word)
{
	uint32_t id = ((bus) << 16) | ((slot) << 11) | ((func) << 8);
	uint32_t addr = 0x80000000 | id | (offset & 0xfc);
	outportl(0xCF8, addr);
	outportl(0xCFC, word);
}


uint16_t pci_get_vendor(uint16_t bus, uint16_t device, uint16_t function)
{
	uint16_t r = pci_read_word(bus,device,function,0);
	return r;
}

uint16_t pci_get_device(uint16_t bus, uint16_t device, uint16_t function)
{
	uint16_t r = pci_read_word(bus,device,function,2);
	return r;
}

void pci_config_write_word(struct pci_device *pdev, uint32_t offset, uint16_t word)
{
	pci_write_word(pdev->bus, pdev->slot, pdev->func, offset, word);
}

void pci_config_write_byte(struct pci_device *pdev, uint32_t offset, uint8_t word)
{
	pci_write_byte(pdev->bus, pdev->slot, pdev->func, offset, word);
}

uint8_t pci_config_read_byte(struct pci_device *pdev, uint32_t offset)
{
	return pci_read_byte(pdev->bus, pdev->slot, pdev->func, offset);
}

uint32_t pci_config_read_dword(struct pci_device *pdev, uint32_t offset)
{
	return pci_read_dword(pdev->bus, pdev->slot, pdev->func, offset);
}

void pci_config_write_dword(struct pci_device *pdev, uint32_t offset, uint32_t dword)
{
	pci_write_dword(pdev->bus, pdev->slot, pdev->func, offset, dword);
}

void pci_info(struct pci_device *pdev, char *str)
{
	printk("[%x:%x]: %s", pdev->vendor, pdev->device, str);
}

void __pci_parse(struct pci_device *pdev)
{
	/* first determine header type */
	uint8_t header = pci_config_read_byte(pdev, 0x0E);
	if (header == 0x00)
	{
		int need_eprom = 1;
		/* determine bars and iobases */
		for(int i = 0; i < 6; i++)
		{
			/* there are 6 BARs read them, check if the first byte is 1 */
			uint32_t r = pci_config_read_dword(pdev, 0x10 + i * 4);
			if (r & 1)
				pdev->iobase = r & 0xFFFFFFFC;
			else {
				pdev->membase = r;
				if (!r) 
					continue;
				need_eprom = 0;
				pci_config_write_dword(pdev, 0x10 + i * 4, 0xffffffff);
				uint32_t size = pci_config_read_dword(pdev, 0x10 + i * 4);
				size &= 0xfffffff0;
				size = ~size;
				size += 1;
				pci_config_write_dword(pdev, 0x10 + i * 4, pdev->membase);
				pdev->memsize = size;
				if (pdev->memsize != 0)
					printk("membase: 0x%x size: %d\n", pdev->membase, size);
			}
		}
		
		if (need_eprom) {
			uint32_t eprom = pci_config_read_dword(pdev, 0x30);
			if (eprom) {
				pci_config_write_dword(pdev, 0x30, 0xffffffff);
				uint32_t size = pci_config_read_dword(pdev, 0x30);
				size &= 0xfffffff0;
				size = ~size;
				size += 1;
				pci_config_write_dword(pdev, 0x30, eprom);
				printk("eprom is 0x%x && size is 0x%x\n", eprom, size);
			}
		}

		/* read IRQ */
		uint8_t irq = pci_config_read_byte(pdev, 0x3C);
		pdev->irq = irq;
	} else {
		pci_info(pdev, "unknown header!\n");
	}
}

void pci_probe()
{
	for (uint32_t bus = 0; bus < 256; bus ++)
	{
		for (uint32_t slot = 0; slot < 32; slot ++)
		{
			for (uint32_t function = 0; function < 8; function ++)
			{
				uint16_t vendor = pci_get_vendor(bus, slot, function);
				if (vendor == 0xffff) continue;
				uint16_t device = pci_get_device(bus, slot, function);
				printk("PCI: [0x%x:0x%x]\n", vendor, device);
				pdevs[pci_devices].vendor = vendor;
				pdevs[pci_devices].device = device;
				pdevs[pci_devices].drv = 0;
				pdevs[pci_devices].bus = bus;
				pdevs[pci_devices].slot = slot;
				pdevs[pci_devices].func = function;
				__pci_parse(&pdevs[pci_devices]);
				pci_devices ++;
			}
		}
	}
}

void pci_init()
{
	pdevs = malloc(sizeof(struct pci_device) * 16);
	pci_probe();
	printk("Found a total of %d PCI devices.\n", pci_devices);
}
