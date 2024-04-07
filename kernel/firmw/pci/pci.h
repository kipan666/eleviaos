#ifndef __FIRMW__PCI_H__
#define __FIRMW__PCI_H__

#include <stdint.h>

int pci_setup();
uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func,
                           uint8_t offset);

#endif // __FIRMW__PCI_H__
