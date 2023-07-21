#ifndef __DEVICES__CPU__SEGMENT__HPP_
#define __DEVICES__CPU__SEGMENT__HPP_

#include <library/types.hpp>

namespace devices {
namespace cpu {
struct Segment64 {
  u16 limit_low;
  u16 base_low;
  u8 base_middle;
  u8 access;
  u8 granularity;
  u8 base_high;
  u32 base_upper;
  u32 reserved;
} __attribute__((packed));
struct Segment32 {
  u16 limit_low;
  u16 base_low;
  u8 base_middle;
  u8 access;
  u8 granularity;
  u8 base_high;
} __attribute__((packed));
} // namespace cpu
} // namespace devices

#endif