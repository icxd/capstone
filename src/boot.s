.set ALIGN,		1<<0				# align loaded modules on page boundaries
.set MEMINFO,	1<<1				# please provide us a memory map
.set VBE_MODE,  1<<2       # VBE mode flag. GRUB will set it for us and provide info about it.
.set FLAGS,		ALIGN | MEMINFO	| VBE_MODE	# multiboot "flag" field
.set MAGIC,		0x1BADB002			# magic number to let the booloader find the header
.set CHECKSUM,	-(MAGIC + FLAGS)	# Checksum of the above

.section .multiboot
.align 4
    .long MAGIC
    .long FLAGS
    .long CHECKSUM
    .long 0, 0, 0, 0, 0
    .long 1
    .long 1024, 768, 32

.section .bootstrap_stack, "aw", @nobits
stack_bottom:
    .skip 65536
stack_top:

.set KERNEL_VIRTUAL_BASE, 0xC0000000
.set KERNEL_PAGE_NUMBER, (KERNEL_VIRTUAL_BASE >> 22)

.section .data
.align 0x1000
.global _boot_page_directory
_boot_page_directory:
.set i, 0x00000083
.rept 0x300
    .long i
    .set i, i + 0x00400000
.endr

.long 0x00000083

.set i, 0xc0400083
.rept 0xFF # 0xFF = (0xffc00000 - 0xc0400000)/0x00400000 + 1
    .long i
    .set i, i+0x00400000
.endr

.section .text
.global _loader
_loader:
    mov $(_boot_page_directory - KERNEL_VIRTUAL_BASE), %ecx
    mov $(_boot_page_directory - KERNEL_VIRTUAL_BASE), %edx
    or $0x00000003, %ecx
    mov %ecx, 0xffc(%edx)

    mov $(_boot_page_directory - KERNEL_VIRTUAL_BASE), %ecx
    mov %ecx, %cr3

    mov %cr4, %ecx
    or 0x10, %ecx
    mov %ecx, %cr4

    mov %cr0, %ecx
    or 0x80000000, %ecx
    mov %ecx, %cr0

    mov $_start, %edx
    jmp *%edx

.global _start
.type _start, @function
_start:
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

    mov %cr0, %eax
    or $1, %al
    mov %eax, %cr0

    mov $stack_top, %esp

    push %ebx
    push %eax
    call kentry

    cli

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
