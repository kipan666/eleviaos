#ifndef __DEVICE__GRAPHIC__TTY__HPP_
#define __DEVICE__GRAPHIC__TTY__HPP_

#include "framebuffer.hpp"
#include <library/types.hpp>

namespace devices {
namespace graphic {
class TTY {
public:
  static void putc(u16 c, int x, int y, u32 color = 0xFFFFFFFF);
  static void putc(u16 c);
  static void print(const char *str);
  static void putn(u64 n, u16 base = 10);

private:
  static inline u16 pos_x = 0;
  static inline u16 pos_y = 0;
  static inline u32 color = 0xFFFFFFFF;
};
} // namespace graphic

} // namespace devices

#endif