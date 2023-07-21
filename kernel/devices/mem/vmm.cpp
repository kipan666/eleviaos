#include "vmm.hpp"

namespace devices {
namespace mem {
void VMM::init() { u64 *page_lvl_4 = (u64 *)create_page_table(); }
/**
 * @brief create page table
 *
 * @return void*
 */
void *VMM::create_page_table() {
  return (void *)(u64)VIRTUAL2PHYS((u64)MEM::calloc(1));
}
} // namespace mem

} // namespace devices