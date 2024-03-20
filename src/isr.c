#include "include/isr.h"
#include "include/idt.h"
#include "include/kernel.h"
#include "include/serial.h"
#include "include/symbols.h"
#include "include/vga.h"

static const char *__exception_labels[] = {
    "[0x00] Divide by Zero Exception",
    "[0x01] Debug Exception",
    "[0x02] Unhandled Non-maskable Interrupt",
    "[0x03] Breakpoint Exception",
    "[0x04] Overflow Exception",
    "[0x05] Bound Range Exceeded Exception",
    "[0x06] Invalid Opcode/Operand Exception",
    "[0x07] Device Unavailable Exception",
    "[0x08] Double Fault",
    "[0x09] Coprocessor Segment Overrun",
    "[0x0A] Invalid TSS Exception",
    "[0x0B] Absent Segment Exception",
    "[0x0C] Stack-segment Fault",
    "[0x0D] General Protection Fault",
    "[0x0E] Page Fault",
    "[0x0F] Inexplicable Error",
    "[0x10] x87 Floating Exception",
    "[0x11] Alignment Check",
    "[0x12] Machine Check",
    "[0x13] SIMD Floating Exception",
    "[0x14] Virtualized Exception",
    "[0x15] Control Protection Exception",
    "[0x16] Inexplicable Error",
    "[0x17] Inexplicable Error",
    "[0x18] Inexplicable Error",
    "[0x19] Inexplicable Error",
    "[0x1A] Inexplicable Error",
    "[0x1B] Inexplicable Error",
    "[0x1C] Hypervisor Intrusion Exception",
    "[0x1D] VMM Communications Exception",
    "[0x1E] Security Exception",
    "[0x1F] Inexplicable Error"};

// from
// https://github.com/austanss/skylight/blob/trunk/glass/src/cpu/interrupts/isr.c
static void print_interrupt_stacktrace(isr_frame_t *frame) {
    char utoa_buffer[67];
    serial_write("\nattempted stacktrace: interrupt during \n");
    u32 ebp = frame->ebp;
    void *eip = (void *)frame->eip;
    for (;;) {
        symbol_t *symbol = null;
        for (int i = 0; __symbol_tab[i].addr != (void *)0xffffffffffffffff;
             i++) {
            if ((u32)eip != (u32)__symbol_tab[i].addr &&
                (u32)eip < (u32)__symbol_tab[i + 1].addr) {
                symbol = (symbol_t *)&__symbol_tab[i];
                serial_write("\t");
                serial_write(utoa((u32)eip, utoa_buffer, 16));
                serial_write(" <");
                serial_write(symbol->name);
                serial_write(">\n");
                break;
            }
        }

        if (symbol == null) {
            serial_write("\t");
            serial_write(utoa((u32)eip, utoa_buffer, 16));
            serial_write(" <unknown>\n");
        }

        if ((u32)eip != frame->eip) ebp = *(u32 *)ebp;
        if (ebp == 0) return;
        eip = (void *)(*(u32 *)(ebp + 8));
    }
}

static void dump_registers(isr_frame_t *frame) {
    char utoa_buffer[67];

    serial_write("\n\nprocess register dump:\n\teax:");
    serial_write(utoa((u32)frame->eax, utoa_buffer, 16));
    serial_write(", ebx: ");
    serial_write(utoa((u32)frame->ebx, utoa_buffer, 16));
    serial_write(", ecx: ");
    serial_write(utoa((u32)frame->ecx, utoa_buffer, 16));
    serial_write(", edx: ");
    serial_write(utoa((u32)frame->edx, utoa_buffer, 16));
    serial_write(", edi: ");
    serial_write(utoa((u32)frame->edi, utoa_buffer, 16));
    serial_write(", esi: ");
    serial_write(utoa((u32)frame->esi, utoa_buffer, 16));
    // serial_write("\n\te8: ");
    // serial_write(utoa((u32)frame->e8, utoa_buffer, 16));
    // serial_write(", e9: ");
    // serial_write(utoa((u32)frame->e9, utoa_buffer, 16));
    // serial_write(", e10: ");
    // serial_write(utoa((u32)frame->e10, utoa_buffer, 16));
    // serial_write(", e11: ");
    // serial_write(utoa((u32)frame->e11, utoa_buffer, 16));
    // serial_write("\n\te12: ");
    // serial_write(utoa((u32)frame->e12, utoa_buffer, 16));
    // serial_write(", e13: ");
    // serial_write(utoa((u32)frame->e13, utoa_buffer, 16));
    // serial_write(", e14: ");
    // serial_write(utoa((u32)frame->e14, utoa_buffer, 16));
    // serial_write(", e15: ");
    // serial_write(utoa((u32)frame->e15, utoa_buffer, 16));
    // serial_write("\n\tcr0: ");
    // serial_write(utoa((u32)frame->cr0, utoa_buffer, 16));
    // serial_write(", cr2: ");
    // serial_write(utoa((u32)frame->cr2, utoa_buffer, 16));
    // serial_write(", cr3: ");
    // serial_write(utoa((u32)frame->cr3, utoa_buffer, 16));
    // serial_write(", cr4: ");
    // serial_write(utoa((u32)frame->cr4, utoa_buffer, 16));
    serial_write("\n\tesp: ");
    serial_write(utoa((u32)frame->esp, utoa_buffer, 16));
    serial_write(", ebp: ");
    serial_write(utoa((u32)frame->ebp, utoa_buffer, 16));
    serial_write(", eflags: ");
    serial_write(utoa((u32)frame->eflags, utoa_buffer, 16));
    serial_write("\n\terror code: b*");
    serial_write(utoa((u32)frame->error_code, utoa_buffer, 2));
    serial_write("\n");
}

static void analyze_page_fault(u32 code) {
    serial_write("\n\npage fault details:\n\tviolation: ");
    serial_write(((code >> 0) & 1) ? "privileged" : "absent");

    serial_write("\n\taccess: ");
    serial_write(((code >> 1) & 1) ? "write" : "read");

    serial_write("\n\torigin: ");
    serial_write(((code >> 2) & 1) ? "user" : "kernel");

    serial_write("\n\tbad entry: ");
    serial_write(((code >> 3) & 1) ? "true" : "false");

    serial_write("\n\texecution: ");
    serial_write(((code >> 4) & 1) ? "true" : "false");

    serial_write("\n\tpkru: ");
    serial_write(((code >> 5) & 1) ? "true" : "false");

    serial_write("\n\tshadow: ");
    serial_write(((code >> 6) & 1) ? "true" : "false");
}

#define PAGE_FAULT_CODE 0x0e

NORETURN void exception_handler(isr_frame_t *);
void exception_handler(isr_frame_t *frame) {
    serial_set_input_masked(true);
    serial_write("FATAL ");
    serial_write(__exception_labels[frame->vector]);
    serial_write(":\n");
    print_interrupt_stacktrace(frame);
    dump_registers(frame);
    if (frame->vector == PAGE_FAULT_CODE) analyze_page_fault(frame->error_code);
    for (;;) {
        asm volatile("cli");
        asm volatile("hlt");
    }
}