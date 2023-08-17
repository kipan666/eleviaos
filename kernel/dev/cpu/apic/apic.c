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
#include <firmw/acpi/acpi.h>
#include <libk/debug/debug.h>
#include <libk/serial.h>

void cpuid(uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
  asm volatile("cpuid"
               : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
               : "0"(*eax), "2"(*ecx));
}

int apic_is_supported() {
  uint32_t eax, edx, unused;
  cpuid(&eax, &unused, &unused, &edx);
  return edx & (1 << 9);
}

void cpuGetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi) {
  asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

void cpuSetMSR(uint32_t msr, uint32_t lo, uint32_t hi) {
  asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

#define APIC_TPR 0x80
#define APIC_DFR 0xE0
#define APIC_LDR 0xD0
#define APIC_SVR 0xF0
#define APIC_EOI 0xB0
#define APIC_ICR_LOW 0x300
#define APIC_ICR_HIGH 0x310
#define APIC_LOGIC_DEST 0xD0
#define APIC_ARBITATION_PRIOR 0x090
#define APIC_IRR 0x200
#define APIC_IEA 0x480

#define APIC_LVT_MASK (1 << 16) && 0b1
#define APIC_LVT_VECTOR 0xFF

#define APIC_ICR_MSG_FIXED (0b000 << 8)
#define APIC_ICR_MSG_LOWEST_PRIOR (0b001 << 8)
#define APIC_ICR_MSG_SMI (0b010 << 8)
#define APIC_ICR_MSG_NMI (0b100 << 8)
#define APIC_ICR_MSG_INIT (0b101 << 8)

#define APIC_ICR_TGM_LEVEL (1 << 15)
#define APIC_ICR_TGM_EDGE (0 << 15)

#define APIC_LINT0 0x350

void apic_write(uint32_t reg, uint32_t value) {
  *((volatile uint32_t *)(local_apic_addr + reg)) = value;
}

uint32_t apic_read(uint32_t reg) {
  return *((volatile uint32_t *)local_apic_addr + reg);
}

void apic_setup() {
  // check is apic supported
  if (apic_is_supported())
    KDEBUG(DEBUG_LEVEL_DEBUG, "APIC supported");

  // check is x2 apic supported
  // uint32_t eax, ebx, ecx, edx;
  // cpuid(&eax, &ebx, &ecx, &edx);
  // if (ecx & (1 << 21))
  //   KDEBUG(DEBUG_LEVEL_DEBUG, "x2APIC supported");

  KDEBUG(DEBUG_LEVEL_DEBUG, "APIC Base: 0x%x", local_apic_addr);

  apic_write(APIC_TPR, 0x00);
  apic_write(APIC_DFR, 0xFFFFFFFF);
  // apic_write(APIC_LDR, 0x0100000);
  apic_write(APIC_SVR, 0xff | 0x100);
  // KDEBUG(0, "ldr : 0x%x", apic_read(APIC_SVR));

  // apic_write(APIC_LOGIC_DEST, 0x01000000);

  // // unmask
  // apic_write(APIC_LINT0, 0x400 | 0x1);

  // apic_write(0x3E0, 3);
  // apic_write(0x380, 0xFFFFFFFF);
  // apic_write(0x320, 0x10000);
  // uint32_t tick = 0xFFFFFFFF - apic_read(0x390);

  // uint16_t divisor = 1193180 / (100000 / 50);
  // outb_(0x43, 0x80 | 0x00 | 0x30);
  // outb_(0x00, divisor & 0xFF);
  // outb_(0x00, (divisor)&0xFF);
  // uint8_t pit_controll_ = inb_(0x61);
  // outb(0x61, pit_controll_ & ~1);
  // outb(0x61, pit_controll_ | 1);

  // while (!(inb_(0x61) & 0x20))
  //   ;

  // // apic_write(0x320, 33 | 0x20000);
  // apic_write(0x3E0, 3);
  // apic_write(0x380, tick / 10);
  // asm("int $3");

  // apic_write(0x360, 0x00000);

  // apic_eoi();
  apic_write(APIC_ICR_HIGH, (0x0 << 24));
  apic_write(APIC_ICR_LOW, 0x29 | 0x0 | 0x00004000 | 0x0);

  KDEBUG(0, "ICR : 0x%x, delivered status 0b%b, read status : 0b%b",
         apic_read(APIC_ICR_LOW), apic_read(APIC_ICR_LOW >> 12) & 0x1,
         (apic_read(APIC_ICR_LOW) >> 16) & 0x3);
  // move to apic 1
  // apic_write(0x20, 0x01000000);
  // apic_write(APIC_LOGIC_DEST, 0x02000000);
  // apic_write(APIC_LINT0, 0x2);

  // apic_write(APIC_ICR_HIGH, (0x1 << 24));
  // apic_write(APIC_ICR_LOW, (0b101 << 8) | (1 << 14));
  // while (apic_read(APIC_ICR_LOW) & (1 << 12))
  //   asm volatile("pause" : : : "memory");

  // apic_write(APIC_ICR_HIGH, (0x1 << 24));
  // apic_write(APIC_ICR_LOW, (0b101 << 8) | (1 << 15));
  // while (apic_read(APIC_ICR_LOW) & (1 << 12))
  //   asm volatile("pause" : : : "memory");

  // apic_write(APIC_ICR_HIGH, (0x1 << 24));
  // apic_write(APIC_ICR_LOW, (0b110 << 8));
  // while (apic_read(APIC_ICR_LOW) & (1 << 12))
  //   asm volatile("pause" : : : "memory");
  // KDEBUG(0, "ICR : 0x%x, delivered status 0b%b, read status : 0b%b",
  //        apic_read(APIC_ICR_LOW), apic_read(APIC_ICR_LOW >> 12) & 0x1,
  //        (apic_read(APIC_ICR_LOW) >> 16) & 0x3);

  // apic_write(0x20, 0x01000000);

  // accept interrupt
  KDEBUG(0, "APIC Version : 0x%x, max LVT : %d, APIC CUR ID: %d",
         apic_read(0x30), ((apic_read(0x30) >> 16) & 0xFF) + 1,
         (uint8_t)(apic_read(0x20) >> 24));

  KDEBUG(0, "error 0b%b", apic_read(0x370));

  __asm__ volatile("sti");
}

void apic_eoi() { apic_write(APIC_EOI, 0); }