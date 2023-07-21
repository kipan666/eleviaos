#ifndef __DEVICES__MEM__MEM_HPP
#define __DEVICES__MEM__MEM_HPP

#include <boot/limine.h>
#include <library/types.hpp>

namespace devices {
namespace mem {

#define ALIGN_UP(addr, align) ((addr) & ~((align)-1))
#define ALIGN_DOWN(addr, align) (((addr) + (align)-1) & ~((align)-1))
#define BLOCK_SIZE 4096

#ifndef PAGGING_LVL_5
#define PHYS2VIRTUAL(addr) (addr + 0xffff800000000000)
#define VIRTUAL2PHYS(addr) (addr - 0xffff800000000000)
#endif
/**
 * @brief Mem class
 */
class MEM {
public:
  static void init(limine_memmap_entry **memmap, u64 memmap_entries);
  static void *alloc(int numblock);
  static void free(void *ptr);
  static void *realloc(void *ptr, int numblock);
  static void *calloc(int numblock);

private:
  static inline u8 *bitmap_;
  static inline u64 bitmap_size_;
};
} // namespace mem
} // namespace devices

#endif