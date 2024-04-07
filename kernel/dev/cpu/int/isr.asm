
%macro pushall 0
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15    
%endmacro

%macro popall 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

extern int_handler
int_common:
    cmp qword [rsp + 24], 0x28
    je .next
    swapgs
    .next:
        pushall
        mov rdi, rsp
        call int_handler
        popall
        add rsp, 16
        cmp qword [rsp + 8], 0x28
        je .next1
        swapgs
    .next1:
        iretq

%macro isr 1
isr_%1:
%if !(%1 == 8 || (%1 >= 10 && %1 <= 14) || %1 == 17 || %1 == 21 || %1 == 29 || %1 == 30)
    push 0
%endif
    push %1
    jmp int_common
%endmacro

%assign i 0
%rep 256
isr i
%assign i i+1
%endrep

[global int_table]
section .data
int_table:
%assign i 0
%rep 256
    dq isr_%+i
%assign i i+1
%endrep

global pit_interrupt
extern pit_handler
extern local_apic_addr
extern time
pit_interrupt:
        push rax
        push rdi

        ; mov eax, dword [time]
        ; inc eax
        ; mov dword [time], eax
        call pit_handler;

        ; Acknowledge interrupt
        mov rdi, [local_apic_addr]
        add rdi, 0xB0
        xor eax, eax
        stosd
        ; mov word [local_apic_addr + 0x0B0], 0

        pop rdi
        pop rax
        iretq

global spurious_interrupt
spurious_interrupt:
        iretq