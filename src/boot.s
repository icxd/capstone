.set MAGIC, 0x1BADB002
.set FLAGS, 0x00000001 | 0x00000002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
    .long MAGIC
    .long FLAGS
    .long CHECKSUM

stack_bottom:
    .skip 1024
stack_top:

.section .text
.global _start
.type _start, @function

_start:
    cli

    lgdt gdt_descriptor
    xor %ax, %ax
    mov %ax, %ds
    ljmp $0x0008, $fix_cs
fix_cs:
    mov $0x0010, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    mov $stack_top, %esp

    pushl %ebx
    pushl %eax
    call kentry

hlt_loop:
    hlt
    jmp hlt_loop

gdt_start:
    .long 0
    .long 0
gdt_code:
    .short 0xFFFF
    .short 0
    .byte 0
    .byte 0b10011010
    .byte 0b11001111
    .byte 0
gdt_data:
    .short 0xFFFF
    .short 0
    .byte 0
    .byte 0b10010010
    .byte 0b11001111
    .byte 0
gdt_end:

gdt_descriptor:
    .short gdt_end - gdt_start - 1
    .long gdt_start

.size _start, . - _start
