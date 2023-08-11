#include "serial.h"

void serial_setup() {
  outb(0x3f8 + 1, 0x00);
  outb(0x3f8 + 3, 0x80);
  outb(0x3f8 + 0, 0x03);
  outb(0x3f8 + 1, 0x00);
  outb(0x3f8 + 3, 0x03);
  outb(0x3f8 + 2, 0xC7);
  outb(0x3f8 + 4, 0x0B);
}

int is_transmit_empty(void) { return inb(0x3f8 + 5) & 0x20; }

// send data to 0x3f8
void serial_send_char(char c) {
  while (is_transmit_empty() == 0)
    ;

  outb(0x3f8, c);
}

// send even more data to 0x3f8
void serial_send_string(char *str) {
  for (int i = 0; str[i] != '\0'; i++)
    serial_send_char(str[i]);
}

void serial_send_number(uint64_t num, int base) {
  char *str = "0123456789ABCDEF";
  char buffer[64];
  int i = 0;
  while (num > 0) {
    buffer[i] = str[num % base];
    num /= base;
    i++;
  }
  buffer[i] = '\0';
  char buffer2[64];
  int j = 0;
  for (i = i - 1; i >= 0; i--) {
    buffer2[j] = buffer[i];
    j++;
  }
  buffer2[j] = '\0';
  serial_send_string(buffer2);
}