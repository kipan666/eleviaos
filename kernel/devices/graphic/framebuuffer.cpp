#include "framebuffer.hpp"

namespace devices {
namespace graphic {
void Framebuffer::init(limine_framebuffer *fbinfo) { m_fb = fbinfo; }
void Framebuffer::putPixel(u64 x, u64 y, u32 color) {
  char *fb = (char *)m_fb->address;
  fb[4 * x + y * m_fb->pitch + 0] = color & 0xFF;
  fb[4 * x + y * m_fb->pitch + 1] = (color >> 8) & 0xFF;
  fb[4 * x + y * m_fb->pitch + 2] = (color >> 16) & 0xFF;
  fb[4 * x + y * m_fb->pitch + 3] = (color >> 24) & 0xFF;

} // 0xAABB
void Framebuffer::putPixel(u64 line, u32 color) {
  char *fb = (char *)m_fb->address;
  fb[line + 0] = color & 0xFF;
  fb[line + 1] = (color >> 8) & 0xFF;
  fb[line + 2] = (color >> 16) & 0xFF;
  fb[line + 3] = (color >> 24) & 0xFF;
}

} // namespace graphic
} // namespace devices
