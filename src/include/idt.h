#ifndef IDT_H
#define IDT_H

#include "types.h"

#define IDT_MAX_DESCRIPTORS 256
#define IDT_CPU_EXCEPTIONS 32

typedef struct idt_entry_t {
    u16 isr_low;   // The lower 16 bits of the ISR's address
    u16 kernel_cs; // The GDT segment selector that the CPU will load into
                   // CS before calling the ISR
    u8 reserved;   // Set to zero
    u8 attributes; // Type and attributes; see the IDT page
    u16 isr_high;  // The higher 16 bits of the ISR's address
} PACKED idt_entry_t;

typedef struct idtr_t {
    u16 limit; // limit size of all idt segments
    u32 base;  // base address of the first idt segment;
} PACKED idtr_t;

void idt_init();
void idt_set_gate(u8, void *, u8);

#endif // !IDT_H