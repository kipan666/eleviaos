#ifndef __DEVICES__MEM__VMM_HPP_]
#define __DEVICES__MEM__VMM_HPP_

#include "mem.hpp"
#include <library/types.hpp>

namespace devices {
namespace mem {
class VMM {
public:
  static void init();
  static void *create_page_table();
};
} // namespace mem

} // namespace devices

#endif