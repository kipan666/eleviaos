#include "tty.hpp"

#include <library/psf.hpp>

extern "C" char _binary_zap_psf_start;
namespace devices {
namespace graphic {

void TTY::putc(u16 c, int x, int y, u32 color) {
  _PSF_FONT *font = (_PSF_FONT *)&_binary_zap_psf_start;
  int bytesperline = (font->width + 7) / 8;
  u8 *glyph = (u8 *)&_binary_zap_psf_start + font->headersize +
              (c > 0 && c < font->numglyph ? c : 0) * font->bytesperglyph;
  int offs = (y * font->height * Framebuffer::getFramebufferInfo()->pitch) +
             (x * font->width * 4);
  int cx, cy, line, mask;
  for (cy = 0; cy < font->height; cy++) {
    line = offs;
    mask = 1 << (font->width - 1);
    for (cx = 0; cx < font->width; cx++) {
      Framebuffer::putPixel(line, *((u8 *)glyph) & mask ? color : 0);
      mask >>= 1;
      line += sizeof(u32);
    }
    glyph += bytesperline;
    offs += Framebuffer::getFramebufferInfo()->pitch;
  }
}

void TTY::putc(u16 c) {
  if (c == '\n') {
    pos_y++;
    pos_x = 0;
    return;
  }
  putc(c, pos_x, pos_y, color);
  pos_x++;
}

void TTY::print(const char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    if (str[i] == '\n') {
      pos_y++;
      pos_x = 0;
      continue;
    }
    putc(str[i], pos_x, pos_y, color);
    pos_x++;
  }
}

void TTY::putn(u64 n, u16 base) {
  char str[64];
  int i = 0;
  do {
    str[i++] = "0123456789abcdef"[n % base];
    n /= base;
  } while (n);
  str[i] = '\0';
  for (int j = 0; j < i / 2; j++) {
    char tmp = str[j];
    str[j] = str[i - j - 1];
    str[i - j - 1] = tmp;
  }
  if (base == 16)
    print("0x");
  print(str);
}

} // namespace graphic
} // namespace devices