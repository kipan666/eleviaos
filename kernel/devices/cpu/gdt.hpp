#ifndef __DEVICES__CPU__GDT__HPP_
#define __DEVICES__CPU__GDT__HPP_

#include <devices/cpu/segment.hpp>
#include <library/types.hpp>

namespace devices {
namespace cpu {
class GDT {
public:
  void addSegment(u16 limit_low, u32 base, u8 access, u8 limit_flags);
  void flush(int cs, int ds);
  static void init();

private:
  int _pos = 0;
  struct GDTPtr {
    u16 limit;
    u64 ptr;
  } __attribute__((packed));
  static inline Segment32 _gdt_segment[10];

}; // namespace cpu
} // namespace cpu
} // namespace devices

#endif