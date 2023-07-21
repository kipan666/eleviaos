#include "memmov.hpp"

namespace library {
namespace string {

void memmov(void *dest, const void *src, size_t n) {
  u8 *d = (u8 *)dest;
  u8 *s = (u8 *)src;
  if (d < s) {
    while (n--)
      *d++ = *s++;
  } else {
    u8 *lasts = s + (n - 1);
    u8 *lastd = d + (n - 1);
    while (n--)
      *lastd-- = *lasts--;
  }
}

} // namespace string
} // namespace library
