.section .text

.extern exception_handler
isr_common_stub:
    pusha
    mov %ds, %ax
    push %eax
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    push %esp

    cld
    call exception_handler

    pop %eax
    pop %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    popa
    add $8, %esp
    iret

.altmacro
.macro isr_err_stub number
.globl isr_stub_\number
isr_stub_\number:
    push $\number
    jmp isr_common_stub
.endm

.macro isr_no_err_stub number
.globl isr_stub_\number
isr_stub_\number:
    push $0
    push $\number
    jmp isr_common_stub
.endm

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub 8
isr_no_err_stub 9
isr_err_stub 10
isr_err_stub 11
isr_err_stub 12
isr_err_stub 13
isr_err_stub 14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub 17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub 30
isr_no_err_stub 31

.altmacro
.macro isr_stub number
    .long isr_stub_\number
.endm

.globl isr_stub_table
isr_stub_table:
    .set i, 0
    .rept 32
        isr_stub %i
        .set i, i + 1
    .endr
