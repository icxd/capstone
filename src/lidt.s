.section .text

.global idt_reload
idt_reload:
    push %ebp
    mov %esp, %ebp
    pushfl

    lidt (%edi)
    sti

    popfl
    popl %ebp
    ret