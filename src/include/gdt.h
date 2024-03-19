#ifndef GDT_H
#define GDT_H

#include "types.h"

#define NULL_SEGMENT 0
#define CODE_SEGMENT 1
#define DATA_SEGMENT 2

typedef struct gdt_t {
    u16 segment_limit; // segment limit first 0-15 bits
    u16 base_low;      // base first 0-15 bits
    u8 base_middle;    // base 16-23 bits
    u8 access;         // access byte
    u8 granularity;    // high 4 bits (flags) low 4 bits (limit 4 last
                       // bits)(limit is 20 bit wide)
    u8 base_high;      // base 24-31 bits
} PACKED gdt_t;

typedef struct gdt_ptr_t {
    u16 limit_size;   // limit size of all gdt segments
    gdt_t *base_addr; // base address of the first gdt segment;
} PACKED gdt_ptr_t;

extern gdt_t gdt_entries[3];
extern gdt_ptr_t gdt_first;

void gdt_init();

#endif // !GDT_H