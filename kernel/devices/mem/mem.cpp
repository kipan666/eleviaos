#include "mem.hpp"

#include <devices/graphic/tty.hpp>
#include <library/string/memset.hpp>

using devices::graphic::TTY;

namespace devices {
namespace mem {

using namespace library::string;

void MEM::init(limine_memmap_entry **memmap, u64 memmap_entries) {
  TTY::print("initialize bitmap\n");
  uptr budy_size = 4 * 1024 * 1024;
  uptr highest_base;
  for (u64 i = 0; i < memmap_entries; i++) {
    limine_memmap_entry *entry = memmap[i];
    if (entry->type != LIMINE_MEMMAP_USABLE)
      continue;
    if (entry->base > highest_base)
      highest_base = entry->base;
  }

  bitmap_size_ = ALIGN_DOWN(ALIGN_UP(highest_base, BLOCK_SIZE) / BLOCK_SIZE / 8,
                            BLOCK_SIZE);

  uptr larger_base;
  for (u64 i = 0; i < memmap_entries; i++) {
    limine_memmap_entry *entry = memmap[i];
    if (entry->type != LIMINE_MEMMAP_USABLE)
      continue;
    if (entry->length > bitmap_size_) {
      larger_base = entry->length;
      break;
    }
  }

  TTY::print("Bitmap size : ");
  TTY::putn(bitmap_size_);
  TTY::print("\nBitmap hosted at base : ");
  TTY::putn(larger_base, 16);

  TTY::print("\n");

  bitmap_ = reinterpret_cast<u8 *>(PHYS2VIRTUAL(larger_base));
  memset(bitmap_, 0, bitmap_size_);
  TTY::print("initialize bitmap done\n");
}
/**
 * @brief alloc memory
 *
 * @param numblock
 * @return void*
 */
void *MEM::alloc(int numblock) {
  u64 free_block = 0;
  // *(bitmap_ + 1) = 1;
  for (u64 i = 0; i < bitmap_size_; i++) {
    if (*(bitmap_ + i) == 0) {
      free_block = i;
      break;
    }
  }
  for (u64 i = 0; i < numblock; i++) {
    *(bitmap_ + free_block + i) = 1;
  }

  return (void *)(u64)(PHYS2VIRTUAL((free_block)*BLOCK_SIZE));
}
void MEM::free(void *ptr) {
  u64 num_pages = ((u64)VIRTUAL2PHYS(ptr) / BLOCK_SIZE);
  for (u64 i = num_pages; i < bitmap_size_; i++) {
    if (*(bitmap_ + i) == 0)
      break;
    *(bitmap_ + i) = 0;
  }
  ptr = (void *)0;
}
void *MEM::realloc(void *ptr, int numblock) {
  u64 num_pages = ((u64)VIRTUAL2PHYS(ptr) / BLOCK_SIZE);
  for (u64 i = num_pages; i < bitmap_size_; i++) {
    if (*(bitmap_ + i) == 0)
      break;
    *(bitmap_ + i) = 0;
  }
  ptr = (void *)0;
  return alloc(numblock);
}
void *MEM::calloc(int numblock) {
  void *ptr = alloc(numblock);
  memset(ptr, 0, numblock * BLOCK_SIZE);
  return ptr;
}
} // namespace mem
} // namespace devices
