#include "pic.hpp"
#include "cpu.hpp"

namespace devices {
namespace cpu {
void PIC::remap() {
  u8 m1 = io::inb(PIC1_DATA);
  u8 m2 = io::inb(PIC2_DATA);

  io::outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
  io::wait();

  io::outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
  io::wait();

  io::outb(PIC1_DATA, 0x20);
  io::outb(PIC2_DATA, 0x28);
  io::wait();

  io::outb(PIC1_DATA, 0x04);
  io::outb(PIC2_DATA, 0x02);
  io::wait();

  io::outb(PIC1_DATA, 0x01);
  io::outb(PIC2_DATA, 0x01);
  io::wait();

  io::outb(PIC1_DATA, 0x00);
  io::outb(PIC2_DATA, 0x00);
  io::wait();

  io::outb(PIC1_DATA, ICW4_8086);
  io::outb(PIC2_DATA, ICW4_8086);
  io::wait();

  io::outb(PIC1_DATA, m1);
  io::outb(PIC2_DATA, m2);
}
} // namespace cpu
} // namespace devices