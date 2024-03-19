.section .text
.global gdt_load 

gdt_load:
    mov 4(%esp), %eax
    lgdt (%eax)

    mov $0x10, %eax
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %fs
    mov %eax, %gs
    mov %eax, %ss
    jmp $0x08, $.longjmp

.longjmp:
    ret