#include "ehci.h"

#include <dev/mem/pmm.h>
#include <firmw/pci/pci.h>
#include <libk/serial.h>
#include <libk/str/memset.h>
#include <stdint.h>

typedef struct {
  volatile uint32_t usbcmd;
  volatile uint32_t usbsts;
  volatile uint32_t usbintr;
  volatile uint32_t frindex;
  volatile uint32_t ctrldssegment;
  volatile uint32_t periodiclistbase;
  volatile uint32_t asynclistaddr;
  volatile uint32_t configflag;
  volatile uint32_t portsc[2];
} __attribute__((packed)) ehci_operation_t;

// queue head transfer
typedef struct {
  uint32_t qhhlp; // que head horiontl link pointer
  uint32_t ch;    // end point characteristics
  uint32_t caps;  // end point capabilities

  volatile uint32_t current_qtd;
  volatile uint32_t next_qtd;
  volatile uint32_t alt_qtd;

  volatile uint32_t status;
  volatile uint32_t buffer[5];

} __attribute__((packed)) ehci_queue_head_t;

// Struktur dasar untuk QH dan QTD
struct QH {
  uint32_t qh_link;
  uint32_t ep_char;
  uint32_t ep_cap;
  uint32_t current_qtd;
  uint32_t next_qtd;
  uint32_t alt_qtd;
  uint32_t status;
  uint32_t buffer[5];
};

struct QTD {
  uint32_t qtd_next;
  uint32_t qtd_alt;
  uint32_t token;
  uint32_t buffer[5];
};

pci_device_t ehci_dev;

void setup_ehci() {

  // TODO: create an list of devices and find the correct device from PCI
  for (int i = 0; i < 32; i++) {
    uint8_t subclass = pci_devices[i].subclass;
    uint8_t class = pci_devices[i].class;
    if (subclass == 0x3 && class == 0xC) {
      ehci_dev = pci_devices[i];
      serial_send_string("\nfound EHCI device\n");
      break;
    }
  }
  if (ehci_dev.bar[0] == 0) {
    serial_send_string("EHCI device not found\n");
    return;
  }

  serial_send_string("found EHCI Base Addr at : 0x");
  serial_send_number(ehci_dev.bar[0], 16);
  serial_send_string("\n");

  //   --start
  uint8_t cap_length = *(uint8_t *)(ehci_dev.bar[0]);
  ehci_operation_t *op = (ehci_operation_t *)(ehci_dev.bar[0] + cap_length);
  serial_send_string("EHCI OP Register base address : 0x");
  serial_send_number((uint32_t)op, 16);
  serial_send_string("\n");

  // first stop ehci
  op->usbcmd &= ~0x1;
  // wait for halt
  serial_send_string("EHCI OP halting... ");
  while (!(op->usbsts & (1 << 12)))
    ;
  serial_send_string("OK\n");

  // reset
  op->usbcmd |= 0x2;
  serial_send_string("EHCI OP resetting... ");
  while (op->usbcmd & 0x2)
    ;
  serial_send_string("OK\n");

  // enable config flag
  serial_send_string("EHCI OP enabling config flag...\n");
  op->configflag = 0x1;

  // activate port 0
  serial_send_string("EHCI OP activating port 0... \n");
  op->portsc[0] = 1 << 2;
  // while (!(op->portsc[0] & 0x1))
  //   ;
  // serial_send_string("OK\n");

  // setup que head
  ehci_queue_head_t *qhead = (ehci_queue_head_t *)pmm_alloc(1);

  op->asynclistaddr = (uint32_t)qhead << 5;
  // op->usbcmd |= 0b01 << 2;
  // op->usbcmd &= ~(1 << 4);
  // op->usbcmd &= ~(1 << 5);
  op->usbintr |= 0x1;
  uint8_t *periodic_list = (uint8_t *)pmm_alloc(1);

  serial_send_string("EHCI OP setting periodic list base address... \n");

  struct QH *qh = (struct QH *)(pmm_alloc(1));
  memset(qh, 0, sizeof(struct QH));
  serial_send_string("QH address: 0x");
  serial_send_number((uint32_t)qh, 16);
  serial_send_string("\n");

  struct QTD *qtd = pmm_alloc(1);
  memset(qtd, 0, sizeof(struct QTD));

  qh->qh_link = (uint32_t)qh + 0x00000001;
  qh->ep_char = 0x00008C00;     // Karakteristik endpoint
  qh->ep_cap = 0x40000000;      // Kapabilitas endpoint
  qh->current_qtd = 0x00000000; // QTD saat ini
  qh->next_qtd = (uint32_t)qtd; // Pointer ke QTD berikutnya
  qh->alt_qtd = 0x00000001;     // Pointer ke QTD alternatif
  qh->status = 0x00000000;      // Status

  qtd->qtd_next = 0x00000001; // Pointer ke qTD berikutnya
  qtd->qtd_alt = 0x00000001;  // Pointer ke qTD alternatif
  qtd->token = 0x80000000;    // Token (status)

  // set to op
  op->asynclistaddr = (uint32_t)qh;
  op->usbcmd |= 0x1;
  op->usbintr |= 0x1;

  // check if ehci running
  if (op->usbcmd & 0x1) {
    if (!(op->usbsts & (1 << 12))) {
      serial_send_string("EHCI OP running... OK\n");
    } else {
      serial_send_string("EHCI OP running... FAILED\n");
    }
  }

  // turn on port 0
  op->portsc[0] = 0x3;
  op->usbcmd |= 0x1;
  // check is port 0 activated
  if (op->portsc[0] & 0x1) {
    serial_send_string("EHCI OP port 0 activated... OK\n");
  } else {
    serial_send_string("EHCI OP port 0 activated... FAILED\n");
  }

  uint8_t buffer[8];
  qtd->buffer[0] = (uint32_t)buffer;

  op->asynclistaddr = (uint32_t)qh;
  op->usbcmd |= 0x1;

  // receive data from port 0 on buffer
  while (!(op->portsc[0] & 0x1))
    ;
  serial_send_string("EHCI OP port 0 data received... OK\n");
  // check buffer is not empty
  if (buffer[0] != 0) {
    serial_send_string("EHCI OP buffer not empty... OK\n");
  } else {
    serial_send_string("EHCI OP buffer not empty... FAILED\n");
  }

  // // verify is ehci already running
  // while (!(op->usbsts & 0x1))
  //   ;
  // serial_send_string("EHCI OP running... OK\n");

  // uint32_t *port0 = (uint32_t *)(op + 0x44);
  // *port0 &= ~(1 << 12);
  // uint8_t *int_ctrl = (uint8_t *)(op->usbcmd + 0xFFFF);
  // int_ctrl = 0x8;
}