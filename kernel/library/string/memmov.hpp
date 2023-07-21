#ifndef __LIBRARY__STRING__MEMMOV_HPP
#define __LIBRARY__STRING__MEMMOV_HPP

#include <library/types.hpp>

namespace library {
namespace string {
void memmov(void *dest, const void *src, size_t n);
}
} // namespace library

#endif