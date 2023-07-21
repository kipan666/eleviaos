#ifndef __DEVICES__CPU__IDT__HPP_
#define __DEVICES__CPU__IDT__HPP_

#include <library/types.hpp>

namespace devices {
namespace cpu {
class IDT {
public:
  void addEntry(u8 id, void *offset, u8 type_attr);
  void flush();
  struct IDTEntry {
    u16 offset_low;
    u16 selector;
    u8 ist;
    u8 type_attr;
    u16 offset_mid;
    u32 offset_high;
    u32 reserved;
  } __attribute__((packed));
  struct IDTPtr {
    u16 limit;
    u64 offset;
  } __attribute__((packed));
  struct register_t {
    u64 r15, r14, r13, r12, r11, r10, r9, r8; // Pushed by pusha.
    u64 rbp, rdi, rsi, rdx, rcx, rbx,
        rax;              // Pushed by the processor automatically.
    u64 int_no, err_code; // Interrupt number and error code (if applicable)
    u64 rip, cs, rflags, rsp, ss; // Pushed by the processor automatically.
  } __attribute__((packed));
  static void init();
  static inline IDTEntry idt_[256];
  static inline IDTPtr idtr;
};

} // namespace cpu
} // namespace devices

#endif