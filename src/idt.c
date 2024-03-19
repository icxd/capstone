#include "include/idt.h"
#include "include/isr.h"

static idt_entry_t idt_entries[IDT_MAX_DESCRIPTORS];
static idtr_t idtr;
static bool vectors[IDT_MAX_DESCRIPTORS];

extern void *isr_stub_table[];
extern void idt_reload(idtr_t *);

void idt_init() {
    idtr.limit_size = (u16)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;
    idtr.base_addr = (u32)&idt_entries[0];

    for (u8 vector = 0; vector < IDT_CPU_EXCEPTIONS; vector++) {
        idt_set_gate(vector, isr_stub_table[vector], IDT_DESCRIPTOR_EXCEPTION);
        vectors[vector] = true;
    }

    idt_reload(&idtr);
}

void idt_set_gate(int vector, u32 isr, u8 flags) {
    idt_entry_t *entry = &idt_entries[vector];

    entry->base_low = isr & 0xffff;
    entry->segment_selector = 0x08;
    entry->zero = 0;
    entry->type = flags;
    entry->base_high = (isr >> 16) & 0xffff;
}