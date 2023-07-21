#include "idt.hpp"
#include "pic.hpp"
#include <devices/graphic/tty.hpp>
#include <stdint.h>
namespace devices {
namespace cpu {

extern "C" void IDTflush(IDT::IDTPtr *);
extern "C" void *int_table[];
using devices::graphic::TTY;

void IDT::addEntry(u8 id, void *offset, u8 type_attr) {
  idt_[id].offset_low = reinterpret_cast<u64>(offset);
  idt_[id].selector = 0x28;
  idt_[id].offset_mid = reinterpret_cast<u64>(offset) >> 16;
  idt_[id].offset_high = reinterpret_cast<u64>(offset) >> 32;
  idt_[id].ist = 0;
  idt_[id].type_attr = type_attr;
  idt_[id].reserved = 0;
  // console.putn(idt_[id].offset_high, 16);
  // console.print("\n");
}
void IDT::flush() {
  idtr.limit = sizeof(idt_) - 1;
  idtr.offset = reinterpret_cast<u64>(&idt_[0]);
  __asm__ volatile("cli");
  __asm__ volatile("lidt %0" : : "memory"(idtr));
  __asm__ volatile("sti");
}
void IDT::init() {
  IDT idt;
  for (int i = 0; i < 256; i++)
    idt.addEntry(i, int_table[i], 0x8E);
  PIC::remap();
  idt.flush();
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

extern "C" void int_handler(IDT::register_t rsp) {

  if (rsp.int_no < 31) {
    TTY::print("\n\n");
    TTY::print(exception_messages[rsp.int_no]);
    TTY::print("\n");
    TTY::print("err code : ");
    TTY::putn(rsp.err_code, 16);
    TTY::print("\n");

    TTY::print("rax : ");
    TTY::putn(rsp.rax, 16);
    TTY::print("  rbx : ");
    TTY::putn(rsp.rbx, 16);
    TTY::print("  rcx : ");
    TTY::putn(rsp.rcx, 16);
    TTY::print("  rdx : ");
    TTY::putn(rsp.rdx, 16);
    TTY::print("\n");
    TTY::print("rsi : ");
    TTY::putn(rsp.rsi, 16);
    TTY::print("  rdi : ");
    TTY::putn(rsp.rdi, 16);
    TTY::print("\n");
    for (;;)
      ;
  }
}
} // namespace cpu
} // namespace devices