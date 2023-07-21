#include "gdt.hpp"

extern "C" void reloadGDT(int cs, int ds);
namespace devices {
namespace cpu {

void GDT::addSegment(u16 limit_low, u32 base, u8 access, u8 limit_flags) {
  Segment32 segment;
  segment.limit_low = limit_low;
  segment.base_low = base;
  segment.base_middle = base >> 16;
  segment.access = access;
  segment.granularity = limit_flags;
  segment.base_high = base >> 24;
  _gdt_segment[_pos] = segment;
  _pos += 1;
}
void GDT::flush(int cs, int ds) {
  GDTPtr gdt_ptr;
  gdt_ptr.limit = sizeof(Segment32) * (_pos + 1) - 1;
  gdt_ptr.ptr = reinterpret_cast<u64>(&_gdt_segment);
  __asm__ volatile("lgdt %0" : : "memory"(gdt_ptr));
  reloadGDT(cs, ds);
}

void GDT::init() {
  GDT gdt;
  // nul descriptor
  gdt.addSegment(0, 0, 0, 0); // 0

  // 16 bit descriptor
  gdt.addSegment(0xffff, 0, 0b10011010, 0x00); // 0x8
  gdt.addSegment(0xffff, 0, 0b10010010, 0x00); // 0x10

  // 32 bit descriptor
  gdt.addSegment(0xffff, 0, 0b10011010, 0xCF); // 0x18
  gdt.addSegment(0xffff, 0, 0b10010010, 0xCF); // 0x20

  // 64 bit descriptor
  gdt.addSegment(0, 0, 0b10011010, 0x20); // 0x28
  gdt.addSegment(0, 0, 0b10010010, 0x00); // 0x30

  // 64 bit user
  gdt.addSegment(0, 0, 0b11111010, 0x20); // 0x38
  gdt.addSegment(0, 0, 0b11110010, 0x00); // 0x40

  // TODO: add TSS
  gdt.flush(0x28, 0x30);
}
} // namespace cpu
} // namespace devices
