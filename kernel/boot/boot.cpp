// ELEVIA (C) 2023
#include "limine.h"
#include <devices/cpu/cpu.hpp>
#include <devices/cpu/gdt.hpp>
#include <devices/cpu/idt.hpp>
#include <devices/graphic/framebuffer.hpp>
#include <devices/graphic/tty.hpp>
#include <devices/mem/mem.hpp>
#include <devices/mem/vmm.hpp>
#include <library/elf.hpp>
#include <library/types.hpp>
#include <trace/trace.hpp>

// limine request
volatile struct limine_framebuffer_request fb_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0, .response = 0};

volatile struct limine_kernel_file_request kernel_request = {
    .id = LIMINE_KERNEL_FILE_REQUEST, .revision = 0, .response = 0};

volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST, .revision = 0, .response = 0};

using devices::graphic::Framebuffer;
using devices::graphic::TTY;
using devices::mem::MEM;
using devices::mem::VMM;

/**
 * @brief The entry point of the kernel
 */
extern "C" void _start(void) {

  limine_framebuffer *fb = fb_request.response->framebuffers[0];
  u64 *fbbuffer = reinterpret_cast<u64 *>(fb->address);
  Framebuffer::init(fb);
  TTY::print("Framebuffer initialized\n");
  limine_file *kernel_file = kernel_request.response->kernel_file;
  trace::Trace::init();

  // GDT & IDT
  devices::cpu::GDT::init();
  TTY::print("Global Descriptor Table initialized\n");
  devices::cpu::IDT::init();
  TTY::print("Interrupt Descriptor Table initialized\n");

  // Memory map
  limine_memmap_entry **memmap_entry = memmap_request.response->entries;
  u64 memmap_entriy_count = memmap_request.response->entry_count;
  MEM::init(memmap_entry, memmap_entriy_count);
  TTY::print("PMM initialized\n");

  //  VMM
  VMM::init();
  TTY::print("VMM initialized\n");

  for (;;) {
  }
}
