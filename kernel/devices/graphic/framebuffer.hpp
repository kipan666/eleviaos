#ifndef __DEVICES__GRAPHIC__FRAMEBUFFER_HPP_
#define __DEVICES__GRAPHIC__FRAMEBUFFER_HPP_

#include <boot/limine.h>
#include <library/types.hpp>

namespace devices {
namespace graphic {
class Framebuffer { // TODO: make double buffering (after malloc has been
                    // implemented)
public:
  static void init(limine_framebuffer *fbinfo);
  static void putPixel(u64 x, u64 y, u32 color);
  static void putPixel(u64 line, u32 color);
  static inline limine_framebuffer *getFramebufferInfo() { return m_fb; }

private:
  static inline limine_framebuffer *m_fb;
};
} // namespace graphic

} // namespace devices

#endif // __DEVICES__GRAPHIC__FRAMEBUFFER_HPP_