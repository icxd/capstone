.section .text

.global idt_reload
    push %ebp
    mov %esp, %ebp
    pushfl
    cli
    lidt (%edi)
    popfl
    popl %ebp
    ret