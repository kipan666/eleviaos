#ifndef __LIBK_STR_MEMSET_H_
#define __LIBK_STR_MEMSET_H_

#include <stdint.h>

void memset(void *ptr, uint8_t value, uint64_t num);
void pmm_free(void *ptr, uint64_t size);

#endif // __LIBK_STR_MEMSET_H_