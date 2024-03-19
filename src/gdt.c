#include "include/gdt.h"

gdt_t gdt_entries[3];
gdt_ptr_t gdt_first;

extern void gdt_load(gdt_t *);

void gdt_init() {
    gdt_entries[NULL_SEGMENT].segment_limit = 0;
    gdt_entries[NULL_SEGMENT].base_low = 0;
    gdt_entries[NULL_SEGMENT].base_middle = 0;
    gdt_entries[NULL_SEGMENT].access = 0;
    gdt_entries[NULL_SEGMENT].granularity = 0;
    gdt_entries[NULL_SEGMENT].base_high = 0;

    gdt_entries[CODE_SEGMENT].segment_limit = 0xfffff;
    gdt_entries[CODE_SEGMENT].base_low = 0;
    gdt_entries[CODE_SEGMENT].base_middle = 0;
    gdt_entries[CODE_SEGMENT].access = 0x9a;
    gdt_entries[CODE_SEGMENT].granularity = 0b11001111;
    gdt_entries[CODE_SEGMENT].base_high = 0;

    gdt_entries[DATA_SEGMENT].segment_limit = 0xfffff;
    gdt_entries[DATA_SEGMENT].base_low = 0;
    gdt_entries[DATA_SEGMENT].base_middle = 0;
    gdt_entries[DATA_SEGMENT].access = 0x92;
    gdt_entries[DATA_SEGMENT].granularity = 0b11001111;
    gdt_entries[DATA_SEGMENT].base_high = 0;

    gdt_first.limit_size = sizeof(gdt_entries) - 1;
    gdt_first.base_addr = (gdt_t *)&gdt_entries;

    gdt_load((gdt_t *)&gdt_first);
}