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

#include "vmm.h"
#include "mem.h"
#include "pmm.h"
#include <libk/debug/debug.h>
#include <libk/serial.h>
#include <libk/str/memset.h>

page_t readCR3(void) {
  uint64_t val;
  __asm__ volatile("mov %%cr3, %0" : "=r"(val));
  return val;
}

// menyiapkan virtual memory dengan ukuran page 4kb
void vmm_setup(void) {
  KDEBUG(1, "Setting up virtual memory");
  page_t pml4 = (vmm_create_page_directory());
  KDEBUG(1, "PML4: 0x%x", higher_half_data_to_phys(pml4));

  for (uint64_t i = 0; i < 4 * GB; i += 0x1000) {
    vmm_map_page(pml4, phys_to_higher_half_data(i), i, 0b11);
  }

  for (uint64_t i = 0; i < 4 * GB; i += 0x1000) {
    vmm_map_page(pml4, i, i, 0b111);
  }

  for (uint64_t i = 0; i < 0x80000000; i += 0x1000) {
    vmm_map_page(pml4, (uint64_t)i + 0xffffffff80000000, i, 0b11);
  }

  vmm_reload(higher_half_data_to_phys(pml4));

  serial_send_string("Page Map Level 4 directory: ");
  serial_send_number(higher_half_data_to_phys((uint64_t)(pml4)), 16);
  serial_send_string("\n");
}

page_t vmm_create_page_directory() {
  uint64_t *page = (uint64_t *)(pmm_alloc(1));
  memset(higher_half_data_to_phys((uint64_t)page), 0, PAGE_SIZE);
  return page;
}

void vmm_map_page(page_t page_dir, uint64_t virt, uint64_t phys, int flags) {
  uint64_t index4 = (virt >> 39) & 0x1ff;
  uint64_t index3 = (virt >> 30) & 0x1ff;
  uint64_t index2 = (virt >> 21) & 0x1ff;
  uint64_t index1 = (virt >> 12) & 0x1ff;

  page_t pml4 = page_dir;
  page_t pdpt = 0;
  page_t pdp = 0;
  page_t pt = 0;

  if (pml4[index4] & 1) {
    pdpt = (page_t)(pml4[index4] & ~(511));
  } else {
    pdpt = vmm_create_page_directory();
    pml4[index4] = higher_half_data_to_phys((uint64_t)pdpt) | flags;
  }

  if (pdpt[index3] & 1) {
    pdp = (page_t)(pdpt[index3] & ~(511));
  } else {
    pdp = (page_t)vmm_create_page_directory();
    pdpt[index3] = higher_half_data_to_phys((uint64_t)pdp) | flags;
  }

  if (pdp[index2] & 1) {
    pt = (page_t)(pdp[index2] & ~(511));
  } else {
    pt = (page_t)vmm_create_page_directory();
    pdp[index2] = higher_half_data_to_phys((uint64_t)pt) | flags;
  }

  pt[index1] = phys | flags;

  asm volatile("invlpg (%0)" ::"r"(virt)
               : "memory"); // flush the entry from TLB
}

void vmm_reload(page_t pml4) {
  asm volatile("mov %0, %%cr3" ::"r"((uint64_t)pml4) : "memory");
}
