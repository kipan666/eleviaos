#include "pci.h"
#include <libk/io.h>
#include <libk/serial.h>

int pci_setup() {
  serial_send_string("pci_setup: ");
  for (int bus = 0; bus < 256; bus++) {
    for (int slot = 0; slot < 32; slot++) {
      for (int func = 0; func < 8; func++) {
        uint16_t vendorID = pciConfigReadWord(bus, slot, func, 0);
        if (vendorID != 0xFFFF) {
          uint16_t deviceID = pciConfigReadWord(bus, slot, func, 2);
          // uint16_t command = pciConfigReadWord(bus, slot, func, 4);
          // uint16_t status = pciConfigReadWord(bus, slot, func, 6);

          serial_send_string("bus: ");
          serial_send_number(bus, 10);
          serial_send_string(", slot: ");
          serial_send_number(slot, 10);
          serial_send_string(", func: ");
          serial_send_number(func, 10);
          serial_send_string(", vendorID: ");
          serial_send_number(vendorID, 10);
          serial_send_string(", deviceID: ");
          serial_send_number(deviceID, 10);
          serial_send_string("\n");
        }
      }
    }
  }
}

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func,
                           uint8_t offset) {
  uint32_t address;
  uint32_t lbus = (uint32_t)bus;
  uint32_t lslot = (uint32_t)slot;
  uint32_t lfunc = (uint32_t)func;
  uint16_t tmp = 0;

  // Create configuration address as per Figure 1
  address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
                       (offset & 0xFC) | ((uint32_t)0x80000000));

  //   // Write out the address
  outl(0xCF8, address);
  // Read in the data
  // (offset & 2) * 8) = 0 will choose the first word of the 32-bit
  tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
  return tmp;
}