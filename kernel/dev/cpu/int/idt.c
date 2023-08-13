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

#include "idt.h"
#include <dev/cpu/apic/apic.h>
#include <dev/cpu/pic/pic.h>
#include <libk/debug/debug.h>
#include <libk/serial.h>

extern void spurious_interrupt();
extern void *int_table[];
extern void pit_interrupt();
extern uint32_t time;
static idt_entry_t idt[256];
static idt_ptr_t idt_ptr;

void idt_setup(void) {
  for (int i = 0; i <= 31; i++)
    idt[i] = add_idt_entry((uint64_t)int_table[i], 0x28, 0, 0x8E);

  pic_remap();
  KDEBUG(1, "Remap and disable PIC done");

  for (int i = 33; i <= 47; i++)
    idt[i] = add_idt_entry((uint64_t)int_table[i], 0x28, 0, 0x8E);

  idt[0x30] = add_idt_entry((uint64_t)int_table[0x30], 0x28, 0, 0x8E);
  idt[0xff] =
      add_idt_entry((uint64_t)(uintptr_t)spurious_interrupt, 0x28, 0, 0x8E);
  // uint8_t val = inb(PIC1_DATA) & ~(1 << 1);
  // outb(PIC1_DATA, val);

  // flush
  idt_ptr.limit = 256 * sizeof(idt_entry_t) - 1;
  idt_ptr.base = (uint64_t)&idt[0];
  asm volatile("lidt %0" : : "m"(idt_ptr));
  // asm("sti");

  // while (inb(0x64) & 0x1)
  //   inb(0x60);

  // while (inb(0x64) & 0x2)
  //   outb(0x60, 0xf4);

  // uint8_t val2 = inb(PIC1_DATA) | (1 << 1);
  // outb(PIC1_DATA, val2);

  // KDEBUG(1, "Keyboard setup done");
}

idt_entry_t add_idt_entry(void *offset, uint16_t selector, uint8_t ist,
                          uint8_t type_attr) {
  idt_entry_t entry;
  entry.offset_low = (uint64_t)offset;
  entry.offset_mid = (uint64_t)offset >> 16;
  entry.offset_high = (uint64_t)offset >> 32;
  entry.selector = selector;
  entry.ist = ist;
  entry.type_attr = type_attr;
  entry.zero = 0;
  return entry;
}

const char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",

    "Page Fault",
    "reserved",
    "x87 FPU Floating Point Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating Point Exception",

    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",

};

extern void int_handler(registers_t rsp) {

  if (rsp.int_no <= 31) {
    KDEBUG(3, "Exception: %s", exception_messages[rsp.int_no]);
    KDEBUG(2, "Error code: %d", rsp.err_code);
    KDEBUG(1, "RIP: %x, CS: %x, RFLAGS: %x, SS: %x", rsp.rip, rsp.cs,
           rsp.rflags, rsp.ss);
    KDEBUG(1, "RAX: %x, RBX: %x, RCX: %x, RDX: %x", rsp.rax, rsp.rbx, rsp.rcx,
           rsp.rdx);
    KDEBUG(1, "RSI: %x, RDI: %x, RBP: %x, RSP: %x", rsp.rsi, rsp.rdi, rsp.rbp,
           rsp.rsp);
    for (;;)
      ;
  } else {
    // start pic
    serial_send_string("isr : ");

    serial_send_number(rsp.int_no, 10);
    serial_send_string("\n");
    // KDEBUG(1, "ISR: 0x%x", local_apic_addr);

    if (rsp.int_no == 33) {
      uint8_t scancode = inb(0x60);
      serial_send_number(scancode, 16);
    } else if (rsp.int_no == 32) {
      time++;
      // serial_send_string("pit\n");
    }
    // check error
    // *(volatile uint32_t *)(local_apic_addr + 0xB0) = 0;
    // *(volatile uint32_t *)(local_apic_addr + 0x420) = 0x2;

    // EOI check
    // // eoi
    apic_eoi();
    // if (rsp.int_no >= 40)
  }
}

extern void pit_handler() {
  serial_send_string("pit\n");
  // ++time;
  apic_eoi();
}