#include "ioapic.h"

#include <firmw/acpi/acpi.h>
#include <libk/debug/debug.h>

#define IOAPICID 0x00
#define IOAPICVER 0x01
#define IOAPICARB 0x02
#define IOAPICREDTBL(n)                                                        \
  (0x10 + 2 * n) // lower-32bits (add +1 for upper 32-bits)

void write_ioapic_register(const uintptr_t apic_base, const uint8_t offset,
                           const uint32_t val) {
  /* tell IOREGSEL where we want to write to */
  *(volatile uint32_t *)(apic_base) = offset;
  /* write the value to IOWIN */
  *(volatile uint32_t *)(apic_base + 0x10) = val;
}

uint32_t read_ioapic_register(const uintptr_t apic_base, const uint8_t offset) {
  /* tell IOREGSEL where we want to read from */
  *(volatile uint32_t *)(apic_base) = offset;
  /* return the data from IOWIN */
  return *(volatile uint32_t *)(apic_base + 0x10);
}

void ioapic_setup() {
  uint32_t ver = read_ioapic_register(io_apic_addr, 1);
  KDEBUG(DEBUG_LEVEL_INFO, "IOAPIC Version: %d", ver & 0xFF);
  KDEBUG(DEBUG_LEVEL_INFO, "IOAPIC Max redirection entry: %d",
         (ver >> 16) & 0xFF);

  //  enable irq 0
  write_ioapic_register(io_apic_addr, IOAPICREDTBL(11), (1 >> 16) | 0x20);
}