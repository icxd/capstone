#include "include/gdt.h"
#include "include/idt.h"
#include "include/isr.h"
#include "include/kernel.h"
#include "include/kheap.h"
#include "include/multiboot.h"
#include "include/pmm.h"
#include "include/process.h"
#include "include/serial.h"
#include "include/types.h"
#include "include/vga.h"
#include "include/vmm.h"

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

void kentry(usz magic, usz addr) {
    s_init();
    multiboot_info_t *mbi = (multiboot_info_t *)addr;
    s_printf("mbi: %p\n", mbi);

    gdt_init();
    idt_init();

    // pmm_parse_memory_map((multiboot_memory_map_t *)(usz)mbi->mmap_addr,
    //                      mbi->mmap_length);
    pmm_init(mbi);
    vmm_init();
    kheap_init();

    pmm_test();
    vmm_test();
    kheap_test();

    // pmm_init(mbi);
    v_init(VGA_YELLOW, VGA_BLACK);

    char utoa_buffer[32];
    v_puts(utoa((u32)magic, utoa_buffer, 16));
    v_puts("\n");
    v_puts(utoa((u32)addr, utoa_buffer, 16));
    v_puts("\n");

    u8 div_by_zero[] = {
        0xba, 0x00, 0x00, 0x00, 0x00, // mov 0x0, edx
        0xb8, 0xfa, 0x00, 0x00, 0x00, // mov 0xfa, eax
        0xb9, 0x00, 0x00, 0x00, 0x00, // mov 0x0, ecx
        0xf7, 0xf1                    // div ecx ---> DIV BY 0
    };
    u8 div_by_not_zero[] = {
        0xba, 0x00, 0x00, 0x00, 0x00, // mov 0x0, edx
        0xb8, 0xfa, 0x00, 0x00, 0x00, // mov 0xfa, eax
        0xb9, 0x00, 0x00, 0x00, 0x01, // mov != 0x0, ecx
        0xf7, 0xf1                    // div ecx ---> DIV BY 0
    };
    // movb   $0x41,0x1000
    u8 write_A_on_1000[] = {0xc6, 0x05, 0x00, 0x10, 0x00, 0x00, 0x41};
    u8 syscall_test[] = {0xb8, 0x02, 0x00, 0x00, 0x00, 0xcd, 0x80};

    load_flat_binary_at(div_by_zero, sizeof(div_by_zero), (void *)0x1000);
    load_flat_binary_at(div_by_not_zero, sizeof(div_by_not_zero),
                        (void *)0x2000);
    load_flat_binary_at(write_A_on_1000, sizeof(write_A_on_1000),
                        (void *)0x3000);
    load_flat_binary_at(syscall_test, sizeof(syscall_test), (void *)0x4000);

    run_flat_binary((void *)0x1000);
    run_flat_binary((void *)0x2000);
    run_flat_binary((void *)0x3000);
    run_flat_binary((void *)0x4000);

    // cpuid_test();
    s_puts("Hello, world!\n");

    v_puts("Hello, world!\n");

    // asm("int $0xe");
}
