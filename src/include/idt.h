#ifndef IDT_H
#define IDT_H

#include "types.h"

#define IDT_MAX_DESCRIPTORS 256
#define IDT_CPU_EXCEPTIONS 32

#define IDT_DESCRIPTOR_X32_TASK 0x05
#define IDT_DESCRIPTOR_X32_INTERRUPT 0x0e
#define IDT_DESCRIPTOR_X32_TRAP 0x0f

#define IDT_DESCRIPTOR_RING1 0x40
#define IDT_DESCRIPTOR_RING2 0x20
#define IDT_DESCRIPTOR_RING3 0x60
#define IDT_DESCRIPTOR_PRESENT 0x80

#define IDT_DESCRIPTOR_EXCEPTION                                               \
    (IDT_DESCRIPTOR_X32_INTERRUPT | IDT_DESCRIPTOR_PRESENT)
#define IDT_DESCRIPTOR_EXTERNAL                                                \
    (IDT_DESCRIPTOR_X32_INTERRUPT | IDT_DESCRIPTOR_PRESENT)

typedef struct idt_entry_t {
    u16 base_low; // lower 16 bits 0-15 of the address to jump to when this
                  // interrupt fires
    u16 segment_selector; // code segment selector in IDT
    u8 zero;              // unused, always be zero
    u8 type;              // types trap, interrupt gates
    u16 base_high;        // upper 16 bits 16-31 of the address to jump to
} PACKED idt_entry_t;

typedef struct idtr_t {
    u16 limit_size; // limit size of all idt segments
    u32 base_addr;  // base address of the first idt segment;
} PACKED idtr_t;

void idt_init();
void idt_set_gate(int, u32, u8);

#endif // !IDT_H