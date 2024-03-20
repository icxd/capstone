#include "include/idt.h"
#include "include/isr.h"

ALIGNED(0x10) static idt_entry_t idt_entries[IDT_MAX_DESCRIPTORS];
static idtr_t idtr;
static bool vectors[IDT_MAX_DESCRIPTORS];

extern void *isr_stub_table[];
// FIXME: extern void idt_reload(idtr_t *);

void idt_init() {
    idtr.base = (u32)&idt_entries[0];
    idtr.limit = (u16)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

    for (u8 vector = 0; vector < IDT_CPU_EXCEPTIONS; vector++) {
        idt_set_gate(vector, isr_stub_table[vector], 0x8e);
        vectors[vector] = true;
    }

    // idt_reload(&idtr);
    asm volatile("lidtl (%0)" : : "r"(&idtr));
}

void idt_set_gate(u8 vector, void *isr, u8 flags) {
    idt_entry_t *entry = &idt_entries[vector];

    entry->isr_low = (u32)isr & 0xffff;
    entry->kernel_cs = 0x08;
    entry->attributes = flags;
    entry->isr_high = (u32)isr >> 16;
    entry->reserved = 0;
}