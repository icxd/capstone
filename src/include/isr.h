#ifndef ISR_H
#define ISR_H

#include "types.h"

typedef struct {
    u32 ds;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 vector, error_code;
    u32 eip, cs, eflags, useresp, ss;
} isr_frame_t;

#endif // !ISR_H