#include "memset.hpp"

namespace library {
namespace string {

void *memset(void *dest, int c, size_t n) {
  unsigned char *p = (unsigned char *)dest;
  while (n--)
    *p++ = (unsigned char)c;
  return dest;
}

} // namespace string
} // namespace library