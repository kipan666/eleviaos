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

#include "stivale2.h"
#include <dev/cpu/apic/apic.h>
#include <dev/cpu/apic/ioapic.h>
#include <dev/cpu/apic/timer.h>
#include <dev/cpu/gdt/gdt.h>
#include <dev/cpu/int/idt.h>
#include <dev/cpu/pic/pic.h>
#include <dev/graphic/fb.h>
#include <dev/initrd/initrd.h>
#include <dev/mem/budy.h>
#include <dev/mem/pmm.h>
#include <dev/mem/vmm.h>
#include <firmw/acpi/acpi.h>
#include <firmw/acpi/madt.h>
#include <firmw/acpi/rsdp.h>
#include <firmw/acpi/rsdt.h>
#include <libk/console/console.h>
#include <libk/debug/debug.h>
#include <libk/io.h>
#include <libk/serial.h>

static uint8_t stack[4096];
static uint64_t *initrd;
uint32_t time = 0;

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
    .flags = (1 << 1) || (1 << 2),
    .tags = (uint64_t)&framebuffer_hdr_tag};

void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id) {
  struct stivale2_tag *current_tag =
      (struct stivale2_tag *)(void *)stivale2_struct->tags;

  for (;;) {
    if (current_tag == 0)
      return 0;

    if (current_tag->identifier == id)
      return current_tag;

    current_tag = (struct stivale2_tag *)(void *)current_tag->next;
  }
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while (n-- != 0) {
    if (*s1 != *s2++)
      return *(unsigned char *)s1 - *(unsigned char *)--s2;
    if (*s1++ == 0)
      break;
  }
  return 0;
}

void _start(struct stivale2_struct *stivale2_struct) {
  serial_setup();

  struct stivale2_struct_tag_modules *modules_info =
      (struct stivale2_struct_tag_modules *)stivale2_get_tag(
          stivale2_struct, STIVALE2_STRUCT_TAG_MODULES_ID);

  if (modules_info->module_count >= 1) {
    serial_send_string("kernel modules found\n");
  }

  initrd_init(modules_info);

  // setup framebuffer
  struct stivale2_struct_tag_framebuffer *framebuffer_info =
      (struct stivale2_struct_tag_framebuffer *)stivale2_get_tag(
          stivale2_struct, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);
  fb_init(framebuffer_info, FB_COLOR_BLACK);
  KDEBUG(DEBUG_LEVEL_INFO, "Graphic initialized");

  // memory
  struct stivale2_struct_tag_memmap *memmap_info =
      (struct stivale2_struct_tag_memmap *)stivale2_get_tag(
          stivale2_struct, STIVALE2_STRUCT_TAG_MEMMAP_ID);

  pmm_setup(memmap_info);
  KDEBUG(DEBUG_LEVEL_INFO, "Physical Memory Allocator initialized");

  vmm_setup();
  KDEBUG(DEBUG_LEVEL_INFO, "Virtual Memory Allocator initialized");

  // gdt
  gdt_setup();
  KDEBUG(DEBUG_LEVEL_INFO, "Global Descriptor Table initialized");

  // IDT
  idt_setup();
  KDEBUG(DEBUG_LEVEL_INFO, "Interrupt Descriptor Table initialized");

  // RSDP
  struct stivale2_struct_tag_rsdp *rsdp_info =
      (struct stivale2_struct_tag_rsdp *)stivale2_get_tag(
          stivale2_struct, STIVALE2_STRUCT_TAG_RSDP_ID);
  KDEBUG(DEBUG_LEVEL_INFO, "RSDP address: 0x%x", rsdp_info->rsdp);

  // ACPI
  acpi_setup(rsdp_info);

  // APIC
  apic_setup();
  // apic_timer_setup();
  ioapic_setup();

  KDEBUG(DEBUG_LEVEL_INFO,
         "Advanced Programmable Interrupt Controller initialized");
  // KDEBUG(0, "Time : %d", time);

  // while (inb(0x64) & 0x1)
  //   inb(0x60);

  // while (inb(0x64) & 0x2)
  //   outb(0x60, 0xf4);

  // // uint8_t val2 = inb(PIC1_DATA) | (1 << 1);
  // // outb(PIC1_DATA, val2);

  // KDEBUG(1, "Keyboard setup done");
  for (;;) {
  }
}
