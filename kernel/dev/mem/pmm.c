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

#include "pmm.h"
#include <libk/debug/debug.h>
#include <libk/serial.h>
#include <libk/str/memset.h>

uint64_t higher_base_length_;
uint64_t bitmap_size_;
uint8_t *bitmap_base_;

void pmm_setup(struct stivale2_struct_tag_memmap *memmap) {
  for (uint64_t i = 0; i < memmap->entries; i++) {
    struct stivale2_mmap_entry *entry = &memmap->memmap[i];
    if (entry->type != STIVALE2_MMAP_USABLE &&
        entry->type != STIVALE2_MMAP_KERNEL_AND_MODULES &&
        entry->type != STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE)
      continue;
    if (entry->base + entry->length > higher_base_length_)
      higher_base_length_ = entry->base + entry->length;
  }
  bitmap_size_ = ALIGN_UP(
      ALIGN_DOWN(higher_base_length_, BLOCK_SIZE) / BLOCK_SIZE / 8, BLOCK_SIZE);
  KDEBUG(DEBUG_LEVEL_INFO, "bitmap size: %dkb", bitmap_size_ / 1024);

  // find base to host bitmap
  for (uint64_t i = 0; i < memmap->entries; i++) {
    struct stivale2_mmap_entry *entry = &memmap->memmap[i];
    if (entry->type != STIVALE2_MMAP_USABLE)
      continue;
    if (entry->length >= bitmap_size_) {
      bitmap_base_ = (uint8_t *)(phys_to_higher_half_data(entry->base));
      entry->base += bitmap_size_;
      entry->length -= bitmap_size_;
      break;
    }
  }
  //   setup bitmap
  KDEBUG(DEBUG_LEVEL_INFO, "bitmap physical memory allocator hosted at: 0x%x",
         bitmap_base_);
  memset((void *)bitmap_base_, 0xFF, bitmap_size_);
  for (uint64_t i = 0; i < memmap->entries; i++) {
    struct stivale2_mmap_entry *entry = &memmap->memmap[i];
    if (entry->type != STIVALE2_MMAP_USABLE)
      continue;

    pmm_free((void *)entry->base, entry->length / BLOCK_SIZE);
  }
  bitmap_base_[0 / 8] |= 1 << (0 % 8);
}

void *pmm_alloc(size_t block) {
  uint64_t start = 0;
  for (uint64_t i = 0; i < higher_base_length_ / BLOCK_SIZE; i++) {
    if (!(bitmap_base_[i / 8] & (1 << (i % 8)))) {
      start = i;
      break;
    }
  }
  if (!start) {
    KDEBUG(DEBUG_LEVEL_ERROR, "out of memory");
  }
  for (size_t i = 0; i < block; i++) {
    bitmap_base_[(start + i) / 8] |= 1 << ((start + i) % 8);
  }

  void *out = (void *)phys_to_higher_half_data((uint64_t)(start * BLOCK_SIZE));
  return out;
}

void pmm_free(void *ptr, size_t size) {
  uint64_t index = ((uint64_t)ptr) / BLOCK_SIZE;
  for (size_t i = 0; i < size; i++)
    bitmap_base_[(index + i) / 8] &= ~(1 << ((index + i) % 8));
}