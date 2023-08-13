/*
  BSD 3-Clause License

  Copyright (c) 2023, Mohammad Arfan
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "acpi.h"
#include "madt.h"
#include "rsdp.h"
#include "rsdt.h"
#include <libk/debug/debug.h>
#include <stddef.h>
#include <stdint.h>

struct ACPI_APIC_ENTRY {
  uint8_t type;
  uint8_t length;
} __attribute__((packed));

struct ACPI_IO_APIC {
  struct ACPI_APIC_ENTRY h;
  uint8_t ioApicId;
  uint8_t reserved;
  uint32_t ioApicAddress;
  uint32_t globalSystemInterruptBase;
} __attribute__((packed));

struct ACPI_INT_SRC {
  struct ACPI_APIC_ENTRY h;
  uint8_t busSource;
  uint8_t irqSource;
  uint32_t globalSystemInterrupt;
  uint16_t flags;
} __attribute__((packed));

uint8_t *local_apic_addr = 0;
uint8_t *io_apic_addr = 0;

int strncmp_(const char *s1, const char *s2, size_t n) {
  while (n-- != 0) {
    if (*s1 != *s2++)
      return *(unsigned char *)s1 - *(unsigned char *)--s2;
    if (*s1++ == 0)
      break;
  }
  return 0;
}

void madt_parse(struct MADT *madt_) {
  // 1. find Local APCI Address
  local_apic_addr = (uint8_t *)(uintptr_t)madt_->localApicAddress;
  KDEBUG(DEBUG_LEVEL_DEBUG, "Local APIC Address: 0x%x", local_apic_addr);

  // parse entries
  uint8_t *ptr = (uint8_t *)(uintptr_t)madt_ + 44;
  uint8_t *ptr_end = ptr + *(uint32_t *)madt_->length;
  while (ptr < ptr_end) {
    uint8_t type = ptr[0];
    uint8_t len = ptr[1];
    switch (type) {
    case 0: {
      KDEBUG(0, "Processor Local APIC -> CPU core ID : %d",
             *(uint8_t *)(ptr + 3));
      break;
    }
    case 1: {
      struct ACPI_IO_APIC *ioapic = (struct ACPI_IO_APIC *)ptr;
      io_apic_addr = (uint8_t *)(uintptr_t)ioapic->ioApicAddress;
      KDEBUG(DEBUG_LEVEL_DEBUG, "IOAPIC: 0x%x", io_apic_addr);
      break;
    }
    case 2: {
      struct ACPI_INT_SRC *int_src = (struct ACPI_INT_SRC *)ptr;
      KDEBUG(DEBUG_LEVEL_DEBUG, "INT SRC: %d, INT SRC: %d",
             int_src->globalSystemInterrupt, int_src->busSource);
      break;
    }
    }
    ptr += len;
  }
}

void acpi_setup(struct stivale2_struct_tag_rsdp *rsdp_info) {
  // 1. parse RSDP
  ACPI_RSDP *rsdp = (ACPI_RSDP *)rsdp_info->rsdp;
  struct MADT *madt_ = 0;

  // 2. parse RSDT
  struct ACPI_RSDT *rsdt = (struct ACPI_RSDT *)rsdp->RsdtAddress;

  for (int i = 0; i < (rsdt->h.Length - sizeof(rsdt->h)) / 4; i++) {
    struct ACPI_RSDT *h = (struct ACPI_RSDT *)rsdt->PointerToOtherSDT[i];
    if (strncmp_(h->h.Signature, "APIC", 4) == 0) {
      kernel_debug_impl(__FILE__, __LINE__, DEBUG_LEVEL_DEBUG,
                        "MADT Found at: 0x%x", h);
      madt_ = (struct MADT *)h;
      break;
    }
  }

  if (!madt_) {
    KDEBUG(DEBUG_LEVEL_ERROR, "Multiple APIC Description Table not found");
    for (;;)
      ;
  }

  // 3. Parse APIC
  madt_parse(madt_);
  KDEBUG(DEBUG_LEVEL_DEBUG, "ACPI Initialized");
}