[bits 64]
global IDTflush

IDTflush:
    cli
    lidt [rdi]
    sti
    ret