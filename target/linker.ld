ENTRY(_start)

SECTIONS {
    . = 0xc0100000;

    kernel_virt_start = .;
    kernel_phys_start = . - 0xc0000000;

    .text : AT(ADDR(.text) - 0xc0000000) {
        *(.multiboot)
        *(.text)
        . = ALIGN(0x1000);
    }

    .rodata : AT(ADDR(.rodata) - 0xc0000000) {
        *(.rodata)
        . = ALIGN(0x1000);
    }

    .data : AT(ADDR(.data) - 0xc0000000) {
        *(.data)
        . = ALIGN(0x1000);
    }

    .bss : AT(ADDR(.bss) - 0xc0000000) {
        *(COMMON)
        *(.bss)
        . = ALIGN(0x1000);
    }

    kernel_virt_end = .;
    kernel_phys_end = . - 0xc0000000;
}