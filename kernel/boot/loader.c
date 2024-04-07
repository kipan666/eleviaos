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

#include "loader.h"

// ini adalah besar dari stack yang akan digunakan oleh kernel
static uint8_t stack[4096];

static struct stivale2_tag l5_tag = {
    .identifier = STIVALE2_HEADER_TAG_5LV_PAGING_ID, .next = 0};

static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    .tag = {.identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID, .next = 0},
    .framebuffer_width = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp = 0};

__attribute__((section(".stivale2hdr"),
               used)) static struct stivale2_header stivale_hdr = {
    .entry_point = 0,
    .stack = (uintptr_t)stack + sizeof(stack),
    .flags = (1 << 1) | (1 << 2),
    .tags = (uint64_t)&framebuffer_hdr_tag};

void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id)
{
    struct stivale2_tag *current_tag =
        (struct stivale2_tag *)(void *)stivale2_struct->tags;

    for (;;)
    {
        if (current_tag == 0)
            return 0;

        if (current_tag->identifier == id)
            return current_tag;

        current_tag = (struct stivale2_tag *)(void *)current_tag->next;
    }
}