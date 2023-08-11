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

#include "apic.h"
#include <dev/mem/mem.h>
#include <libk/debug/debug.h>
#include <libk/serial.h>

void cpuid(uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
  asm volatile("cpuid"
               : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
               : "0"(*eax), "2"(*ecx));
}

void cpuGetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi) {
  asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

void cpuSetMSR(uint32_t msr, uint32_t lo, uint32_t hi) {
  asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

int apic_is_supported() {
  uint32_t eax, edx, unused;
  cpuid(&eax, &unused, &unused, &edx);
  return edx & (1 << 9);
}

void apic_setup(void) {
  if (apic_is_supported())
    KDEBUG(1, "APIC supported");

  //   get apic addr
  uint32_t eax, edx;
  cpuGetMSR(0x1B, &eax, &edx);
  uintptr_t apic_addr = (eax & 0xfffff000);
  KDEBUG(1, "APIC addr: 0x%x", apic_addr);

  edx = 0;
  eax = (apic_addr & 0xfffff0000) | 0x800;
  cpuSetMSR(0x1B, eax, edx);
  uint32_t *apic_base = (uint32_t *)phys_to_higher_half_data(apic_addr);

  uint32_t *svr = apic_base + 0xF0;
  *svr |= (1 << 8);

  //   IOACPI Setup
  // get number entries support of IOAPIC
  *((uint32_t *)apic_base + 0x0) = 0x1;
  uint32_t i = *((uint32_t *)(apic_base + 0x10));
  unsigned int max = ((i >> 16) & 0xff) + 1;
}