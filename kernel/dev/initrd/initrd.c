/*
  BSD 3-Clause License

  Copyright (c) 2023, Mohammad Arfan
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "initrd.h"
#include <libk/fs/tar.h>
#include <libk/serial.h>
#include <stddef.h>

static uint8_t *initrd_base_;
static uint8_t *initrd_end_;

int strcmp(const char *str1, const char *str2) {
  while (*str1 && *str2) {
    if (*str1 != *str2) {
      return 0;
    }
    str1++;
    str2++;
  }
  return 1;
}

int oct2bin(unsigned char *str, int len) {
  int n = 0;
  unsigned char *c = str;
  while (len-- > 0) {
    n *= 8;
    n += *c - '0';
    c++;
  }
  return n;
}

void initrd_init(struct stivale2_struct_tag_modules *modules_tag) {
  for (uint64_t i = 0; i < modules_tag->module_count; i++) {
    struct stivale2_module *module = &modules_tag->modules[i];
    if (strcmp(module->string, "boot:///initrd.tar")) {
      serial_send_string("\ninitrd found\n");
      initrd_base_ = (uint8_t *)module->begin;
      initrd_end_ = (uint8_t *)module->end;
    }
  }
}

// find file in initrd
// TODO: add support for recursive search
char *initrd_find_file(const char *name) {
  uint8_t *addr = initrd_base_;
  TarHeader *header = (TarHeader *)addr;
  while (strcmp(header->ustar, "ustar")) {
    int size = oct2bin(header->size, 11);
    if (strcmp(header->filename, name)) {
      serial_send_string(header->filename);
      serial_send_string(" loaded from rootdir\n");
      if (header->typeflag[0] == '5') {
        uint8_t *subaddr = addr + 512;
        header = (TarHeader *)subaddr;
        serial_send_string(header->filename);
        serial_send_string(" loaded from subdir\n");
        char *out = subaddr + 512;
        return out;
      }
      char *out = addr + 512;
      return out;
    }
    addr += (((size + 511) / 512) + 1) * 512;
    header = (TarHeader *)addr;
  }
}