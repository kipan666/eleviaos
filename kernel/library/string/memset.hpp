#ifndef __LIBRARY__STRING__MEMSET_HPP
#define __LIBRARY__STRING__MEMSET_HPP

#include <library/types.hpp>

namespace library {
namespace string {
void *memset(void *dest, int c, size_t n);
} // namespace string

} // namespace library

#endif