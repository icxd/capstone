#ifndef ISR_H
#define ISR_H

#include "types.h"

typedef struct {
    struct {
        u32 cr4;
        u32 cr3;
        u32 cr2;
        u32 cr0;
    } control_registers;

    struct {
        u32 e15;
        u32 e14;
        u32 e13;
        u32 e12;
        u32 e11;
        u32 e10;
        u32 e9;
        u32 e8;
        u32 edi;
        u32 esi;
        u32 edx;
        u32 ecx;
        u32 ebx;
        u32 eax;
    } general_registers;

    struct {
        u32 ebp;
        u32 vector;
        u32 error_code;
        u32 eip;
        u32 cs;
        u32 eflags;
        u32 esp;
        u32 dss;
    } base_frame;
} isr_frame_t;

#endif // !ISR_H