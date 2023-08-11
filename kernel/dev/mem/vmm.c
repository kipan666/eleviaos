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

#define GB 0x40000000UL

uint64_t readCR3(void) {
  uint64_t val;
  __asm__ volatile("mov %%cr3, %0" : "=r"(val));
  return val;
}

void vmm_setup(void) {
  uintptr_t cr3 = (uintptr_t)readCR3();
  uint64_t *root_dir = (uint64_t *)((cr3 >> 12) << 12);
  KDEBUG(1, "Page Map Level 4 directory: 0x%x", &root_dir);
  serial_send_number((uint64_t)higher_half_data_to_phys(&root_dir), 16);
  serial_send_string("\n");
}

uint64_t *vmm_create_page_directory() {
  uint64_t *page = (uint64_t *)pmm_alloc(1);
  memset(VIRT2PHYS((uint64_t)page), 0, PAGE_SIZE);
  return page;
}

uint64_t *vmm_get_page_level(uint64_t *page_dir, uint64_t index, int flags) {
  if (page_dir[index] & 1) {
    // KDEBUG(0, "Page already allocated for index %d", index);
    return (uint64_t *)(page_dir[index] & ~(511));
  } else {
    // KDEBUG(0, "Allocating page for index %d", index);
    page_dir[index] = VIRT2PHYS((uint64_t)pmm_alloc(1)) | flags;
    return (uint64_t *)(page_dir[index] & ~(511));
  }
}

void vmm_map_page(uint64_t *page_dir, uint64_t virt, uint64_t phys, int flags) {
  uintptr_t index4 = virt >> 39 & 0x1ff;
  uintptr_t index3 = virt >> 30 & 0x1ff;
  uintptr_t index2 = virt >> 21 & 0x1ff;
  uintptr_t index1 = virt >> 12 & 0x1ff;

  uint64_t *pml4 = page_dir;
  uint64_t *pdpt = 0;
  uint64_t *pd = 0;
  uint64_t *pt = 0;

  // pml4 to pdpt
  if (pml4[index4] & 1) {
    pdpt = (uint64_t *)(pml4[index4] & ~(511));
  } else {
    pdpt = (uint64_t *)pmm_alloc(1);
    memset(VIRT2PHYS((uint64_t)pdpt), 0, PAGE_SIZE);
    pml4[index4] = VIRT2PHYS((uint64_t)pdpt) | flags;
  }

  // pdpt to pd
  if (pdpt[index3] & 1) {
    pd = (uint64_t *)(pdpt[index3] & ~(511));
  } else {
    pd = (uint64_t *)pmm_alloc(1);
    memset(VIRT2PHYS((uint64_t)pd), 0, PAGE_SIZE);
    pdpt[index3] = VIRT2PHYS((uint64_t)pd) | flags;
  }

  // pd to pt
  if (pd[index2] & 1) {
    pt = (uint64_t *)(pd[index2] & ~(511));
  } else {
    pt = (uint64_t *)pmm_alloc(1);
    memset(VIRT2PHYS((uint64_t)pt), 0, PAGE_SIZE);
    pd[index2] = VIRT2PHYS((uint64_t)pt) | flags;
  }

  // pt to phys
  // if (pt[index1] & 1) {
  // KDEBUG(0, "Page already mapped");
  // } else {
  pt[index1] = phys | flags;
  // }

  asm volatile("invlpg (%0)" ::"r"(virt)
               : "memory"); // flush the entry from TLB
}
