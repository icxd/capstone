#include "include/gdt.h"
#include "include/idt.h"
#include "include/isr.h"
#include "include/kernel.h"
#include "include/serial.h"
#include "include/types.h"
#include "include/vga.h"

void print_eax(u32 eax) {
    u32 step_id, model, family_id, proc_type, ext_mod_id, ext_fam_id;
    step_id = model = family_id = proc_type = ext_mod_id = ext_fam_id = eax;

    step_id &= (2 << 3) - 1; // bits 0-3
    model >>= 4;             // bits 4-7
    model &= (2 << 3) - 1;
    family_id >>= 8; // bits 8-11
    family_id &= (2 << 3) - 1;
    proc_type >>= 12; // bits 12-13
    proc_type &= (2 << 1) - 1;
    ext_mod_id >>= 16; // bits 16-19
    ext_mod_id &= (2 << 3) - 1;
    ext_fam_id >>= 20; // bits 20-27
    ext_fam_id &= (2 << 7) - 1;

    v_puts("\nEAX :-");
    v_puts("\n\tStepping ID: ");
    v_puti(step_id);
    v_puts("\n\tModel: ");
    v_puti(model);
    v_puts("\n\tFamily ID: ");
    v_puti(family_id);
    v_puts("\n\tProcessor Type: ");
    v_puti(proc_type);
    v_puts("\n\tExtended Model ID: ");
    v_puti(ext_mod_id);
    v_puts("\n\tExtended Family ID: ");
    v_puti(ext_fam_id);
}

void print_ebx(u32 ebx) {
    u32 brand_index, cache_line_size, max_addr_id, init_apic_id;
    brand_index = cache_line_size = max_addr_id = init_apic_id = 0;
    char *bytes = (char *)&ebx;

    brand_index = bytes[0];     // bits 0-7
    cache_line_size = bytes[1]; // bits 8-15
    max_addr_id = bytes[2];     // bits 16-23
    init_apic_id = bytes[3];    // bits 24-31

    v_puts("\nEBX :-");
    v_puts("\n\tBrand Index: ");
    v_puti(brand_index);
    v_puts("\n\tCache Line Size: ");
    v_puti(cache_line_size);
    v_puts("\n\tMax Addressable ID for Logical Processors: ");
    v_puti(max_addr_id);
    v_puts("\n\tInitial APIC ID: ");
    v_puti(init_apic_id);
}

void print_edx(u32 edx) {
    v_puts("\nEDX :-");
    v_puts("\n\tbit-31 [ ");
    v_putb(edx);
    v_puts(" ] bit-0");
    v_puts("\n\tBit 0 : FPU-x87 FPU on Chip");
    v_puts("\n\tBit 1 : VME-Virtual-8086 Mode Enhancement");
    v_puts("\n\tBit 2 : DE-Debugging Extensions");
    v_puts("\n\tBit 3 : PSE-Page Size Extensions");
    v_puts("\n\tBit 4 : TSC-Time Stamp Counter");
    v_puts("\n\tBit 5 : MSR-RDMSR and WRMSR Support");
    v_puts("\n\tBit 6 : PAE-Physical Address Extensions");
}

void cpuid_test() {
    u32 eax, ebx, ecx, edx;

    v_puts("CPUID EAX = 01H");

    cpuid(0x01, &eax, &ebx, &ecx, &edx);

    print_eax(eax);
    print_ebx(ebx);
    print_edx(edx);
}

void kentry() {
    gdt_init();
    idt_init();
    serial_init();

    v_init(VGA_YELLOW, VGA_BLACK);

    10 / 0;

    // cpuid_test();
    serial_write("Hello, world!\n");
}