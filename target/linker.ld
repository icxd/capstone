ENTRY(_start)

SECTIONS {
    . = 1M;

    kernel_virt_start = .;
    kernel_phys_start = .;

    .text BLOCK(4K) : ALIGN(4K) {
        *(.multiboot)
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K) {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K) {
        *(COMMON)
        *(.bss)
    }

    kernel_virt_end = .;
    kernel_phys_end = .;
}