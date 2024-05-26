#include "pci.h"
#include <libk/io.h>
#include <libk/serial.h>

pci_device_t pci_devices[32];
int cur_pci_device_num = 0;

int pci_setup() {
  serial_send_string("\npci_setup: \n");
  for (int bus = 0; bus < 256; bus++) {
    for (int slot = 0; slot < 32; slot++) {
      for (int func = 0; func < 8; func++) {
        uint16_t vendorID = pciConfigReadWord(bus, slot, func, 0);
        if (vendorID != 0xFFFF) {
          uint16_t deviceID = pciConfigReadWord(bus, slot, func, 2);
          uint16_t command = pciConfigReadWord(bus, slot, func, 4);
          uint16_t status = pciConfigReadWord(bus, slot, func, 6);
          uint8_t rev_id = pciConfigReadWord(bus, slot, func, 8) & 0xFF;
          uint8_t prog_if = pciConfigReadWord(bus, slot, func, 9) & 0xFF;
          uint8_t subclass = pciConfigReadWord(bus, slot, func, 10) & 0xFF;
          uint8_t class = pciConfigReadWord(bus, slot, func, 10) >> 8;
          uint8_t cache_line_size =
              pciConfigReadWord(bus, slot, func, 12) & 0xFF;
          uint8_t latency_timer = pciConfigReadWord(bus, slot, func, 13) & 0xFF;
          uint8_t header_type = pciConfigReadWord(bus, slot, func, 14) & 0xFF;

          // currently just focussed on header type 0, for now skip another
          // header type 0x0
          if (header_type != 0) {
            continue;
          }

          uint8_t bist = pciConfigReadWord(bus, slot, func, 15) & 0xFF;
          uint32_t bar[6];
          for (int i = 0; i < 6; i++) {
            bar[i] = pciConfigReadWord(bus, slot, func, 16 + i * 4) & 0xFFFF;
            bar[i] |= pciConfigReadWord(bus, slot, func, 16 + i * 4 + 2) << 16;
          }
          uint32_t cardbus_cis_pointer =
              pciConfigReadWord(bus, slot, func, 41) & 0xFFFF;
          cardbus_cis_pointer |= pciConfigReadWord(bus, slot, func, 43) << 16;
          uint16_t subsystem_vendor_id = pciConfigReadWord(bus, slot, func, 44);
          uint16_t subsystem_id = pciConfigReadWord(bus, slot, func, 46);
          uint32_t expansion_rom_base_address =
              pciConfigReadWord(bus, slot, func, 48) & 0xFFFF;
          expansion_rom_base_address |= pciConfigReadWord(bus, slot, func, 50)
                                        << 16;
          uint8_t capabilities_pointer =
              pciConfigReadWord(bus, slot, func, 52) & 0xFF;
          uint8_t interrupt_line =
              pciConfigReadWord(bus, slot, func, 60) & 0xFF;
          uint8_t interrupt_pin = pciConfigReadWord(bus, slot, func, 61) & 0xFF;
          uint8_t min_grant = pciConfigReadWord(bus, slot, func, 62) & 0xFF;
          uint8_t max_latency = pciConfigReadWord(bus, slot, func, 63) & 0xFF;

          pci_devices[cur_pci_device_num].vendorID = vendorID;
          pci_devices[cur_pci_device_num].deviceID = deviceID;
          pci_devices[cur_pci_device_num].command = command;
          pci_devices[cur_pci_device_num].status = status;
          pci_devices[cur_pci_device_num].rev_id = rev_id;
          pci_devices[cur_pci_device_num].prog_if = prog_if;
          pci_devices[cur_pci_device_num].subclass = subclass;
          pci_devices[cur_pci_device_num].class = class;
          pci_devices[cur_pci_device_num].cache_line_size = cache_line_size;
          pci_devices[cur_pci_device_num].latency_timer = latency_timer;
          pci_devices[cur_pci_device_num].header_type = header_type;
          pci_devices[cur_pci_device_num].bist = bist;
          for (int i = 0; i < 6; i++) {
            pci_devices[cur_pci_device_num].bar[i] = bar[i];
          }
          pci_devices[cur_pci_device_num].cardbus_cis_pointer =
              cardbus_cis_pointer;
          pci_devices[cur_pci_device_num].subsystem_vendor_id =
              subsystem_vendor_id;
          pci_devices[cur_pci_device_num].subsystem_id = subsystem_id;
          pci_devices[cur_pci_device_num].expansion_rom_base_address =
              expansion_rom_base_address;
          pci_devices[cur_pci_device_num].capabilities_pointer =
              capabilities_pointer;
          pci_devices[cur_pci_device_num].interrupt_line = interrupt_line;
          pci_devices[cur_pci_device_num].interrupt_pin = interrupt_pin;
          pci_devices[cur_pci_device_num].min_grant = min_grant;
          pci_devices[cur_pci_device_num].max_latency = max_latency;
          cur_pci_device_num++;

          serial_send_string("bus: ");
          serial_send_number(bus, 10);
          serial_send_string(", slot: ");
          serial_send_number(slot, 10);
          serial_send_string(", func: ");
          serial_send_number(func, 10);
          serial_send_string(", class: 0x");
          serial_send_number(pci_devices[cur_pci_device_num - 1].class, 16);
          serial_send_string(", subclass: 0x");
          serial_send_number(pci_devices[cur_pci_device_num - 1].subclass, 16);
          serial_send_string(", bar 0: 0x");
          serial_send_number(bar[0], 16);
          serial_send_string(", header type: 0x");
          serial_send_number(header_type, 16);
          serial_send_string(", IRQ : ");
          serial_send_number(interrupt_line, 10);
          serial_send_string(", PIN : ");
          serial_send_number(interrupt_pin, 10);
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