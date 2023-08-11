#include "memset.h"

void memset(void *ptr, uint8_t value, uint64_t num) {
  uint8_t *ptr_ = (uint8_t *)ptr;
  for (uint64_t i = 0; i < num; i++) {
    ptr_[i] = value;
  }
}