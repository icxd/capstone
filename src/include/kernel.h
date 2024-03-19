#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

u32 strlen(const char *);
u32 digit_count(int);
void itoa(int, char *);
const char *utoa(u32, char *, u8);

void cpuid(u32, u32 *, u32 *, u32 *, u32 *);

u8 inb(u16);
void outb(u16, u8);

void wait_for_io(u32);
void sleep(u32);

#endif // !KERNEL_H