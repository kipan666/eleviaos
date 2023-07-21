#ifndef __DEVICES__CPU__CPU__HPP_
#define __DEVICES__CPU__CPU__HPP_

#include <library/types.hpp>

namespace devices {
namespace cpu {
namespace io {
inline void outb(u16 port, u8 value) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

inline u8 inb(u16 port) {
  u8 ret;
  __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

inline void wait() { __asm__ volatile("outb %%al, $0x80" : : "a"(0)); }

} // namespace io

} // namespace cpu
} // namespace devices

#endif