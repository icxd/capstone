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
    serial_write("Attempted Stacktrace:\n");
    u32 ebp = frame->ebp;
    void *eip = (void *)frame->eip;
    for (;;) {
        symbol_t *symbol = null;
        for (int i = 0; __symbol_tab[i].addr != (void *)0xffffffffffffffff;
             i++) {
            if ((u32)eip != (u32)__symbol_tab[i].addr &&
                (u32)eip < (u32)__symbol_tab[i + 1].addr) {
                symbol = (symbol_t *)&__symbol_tab[i];
                serial_write("    \033[33m0x");
                serial_write(utoa((u32)eip, utoa_buffer, 16));
                serial_write("\033[0m\t\t <\033[34m");
                serial_write(symbol->name);
                serial_write("\033[0m>\n");
                break;
            }
        }

        if (symbol == null) {
            serial_write("    \033[33m0x");
            serial_write(utoa((u32)eip, utoa_buffer, 16));
            serial_write("\033[0m\t\t <\033[31munknown\033[0m>\n");
        }

        if ((u32)eip != frame->eip) ebp = *(u32 *)ebp;
        if (ebp == 0) return;
        eip = (void *)(*(u32 *)(ebp + 8));
    }
}

static void dump_registers(isr_frame_t *frame) {
    char utoa_buffer[67];

    serial_write("\nProcess Register Dump:\n    EAX: \033[33m0x");
    serial_write(utoa((u32)frame->eax, utoa_buffer, 16));
    serial_write("\033[0m, EBX: \033[33m0x");
    serial_write(utoa((u32)frame->ebx, utoa_buffer, 16));
    serial_write("\033[0m, ECX: \033[33m0x");
    serial_write(utoa((u32)frame->ecx, utoa_buffer, 16));
    serial_write("\033[0m, EDX: \033[33m0x");
    serial_write(utoa((u32)frame->edx, utoa_buffer, 16));
    serial_write("\033[0m, EDI: \033[33m0x");
    serial_write(utoa((u32)frame->edi, utoa_buffer, 16));
    serial_write("\033[0m, ESI: \033[33m0x");
    serial_write(utoa((u32)frame->esi, utoa_buffer, 16));
    serial_write("\033[0m\n    ESP: \033[33m0x");
    serial_write(utoa((u32)frame->esp, utoa_buffer, 16));
    serial_write("\033[0m, EBP: \033[33m0x");
    serial_write(utoa((u32)frame->ebp, utoa_buffer, 16));
    serial_write("\033[0m, EFLAGS: \033[33m0x");
    serial_write(utoa((u32)frame->eflags, utoa_buffer, 16));
    serial_write("\033[0m\n    Error Code: \033[33m0b");
    serial_write(utoa((u32)frame->error_code, utoa_buffer, 2));
    serial_write("\033[0m\n");
}

static void analyze_page_fault(u32 code) {
    serial_write("\nPage Fault Details:\n    Violation:\t");
    serial_write(((code >> 0) & 1) ? "\033[34mPrivileged\033[0m"
                                   : "\033[35mAbsent\033[0m");

    serial_write("\n    Access:\t");
    serial_write(((code >> 1) & 1) ? "\033[34mWrite\033[0m"
                                   : "\033[35mRead\033[0m");

    serial_write("\n    Origin:\t");
    serial_write(((code >> 2) & 1) ? "\033[34mUser\033[0m"
                                   : "\033[35mKernel\033[0m");

    serial_write("\n    Bad Entry:\t");
    serial_write(((code >> 3) & 1) ? "\033[32mTrue\033[0m"
                                   : "\033[31mFalse\033[0m");

    serial_write("\n    Execution:\t");
    serial_write(((code >> 4) & 1) ? "\033[32mTrue\033[0m"
                                   : "\033[31mFalse\033[0m");

    serial_write("\n    PKRU:\t");
    serial_write(((code >> 5) & 1) ? "\033[32mTrue\033[0m"
                                   : "\033[31mFalse\033[0m");

    serial_write("\n    Shadow:\t");
    serial_write(((code >> 6) & 1) ? "\033[32mTrue\033[0m"
                                   : "\033[31mFalse\033[0m");
}

#define PAGE_FAULT_CODE 0x0e

NORETURN void interrupt_handler(isr_frame_t *);
void interrupt_handler(isr_frame_t *frame) {
    serial_set_input_masked(true);
    serial_write("\033[31;1mFATAL \033[0;0m");
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