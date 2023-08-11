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

#include "gdt.h"
#include <libk/debug/debug.h>

extern void reloadGDT(int cs, int ds);

static gdt_entry_t gdt_entries[9];
static gdt_ptr_t gdt_ptr;

void gdt_setup() {
  gdt_entries[0] = gdt_make_entry(0, 0, 0, 0); // 0x00 null segment
                                               //   16 bit
  gdt_entries[1] =
      gdt_make_entry(0, 0xFFFF, 0x9A, 0x00); // 0x08 16 bit code segment
  gdt_entries[2] =
      gdt_make_entry(0, 0xFFFF, 0x92, 0x00); // 0x10 16 bit data segment
  //   32 bit
  gdt_entries[3] =
      gdt_make_entry(0, 0xFFFF, 0x9A, 0xCF); // 0x18 32 bit code segment
  gdt_entries[4] =
      gdt_make_entry(0, 0xFFFF, 0x92, 0xCF); // 0x20 32 bit data segment
  // 64 bit
  gdt_entries[5] = gdt_make_entry(0, 0, 0x9A, 0x20); // 0x28 64 bit code segment
  gdt_entries[6] = gdt_make_entry(0, 0, 0x92, 0x00); // 0x30 64 bit data segment
  //   64 bit userspace
  gdt_entries[7] = gdt_make_entry(0, 0, 0xFA, 0x20); // 0x38 64 bit code segment
  gdt_entries[8] = gdt_make_entry(0, 0, 0xF2, 0x00); // 0x40 64 bit data segment
  gdt_ptr.limit = sizeof(gdt_entry_t) * (9 + 1) - 1;
  gdt_ptr.base = (uint64_t)&gdt_entries;
  gdt_flush(gdt_ptr);
  reloadGDT(0x28, 0x30);
}

gdt_entry_t gdt_make_entry(uint32_t base, uint16_t limit, uint8_t access,
                           uint8_t flags) {
  gdt_entry_t entry;
  entry.base_low = base & 0xFFFF;
  entry.base_middle = (base >> 16) & 0xFF;
  entry.base_high = (base >> 24) & 0xFF;
  entry.limit_low = limit;
  entry.flags = flags;
  entry.access = access;
  return entry;
}

void gdt_flush(gdt_ptr_t gdt_ptr) {
  __asm__ volatile("lgdt %0" : : "memory"(gdt_ptr));
}