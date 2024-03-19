#include "include/kernel.h"

u32 strlen(const char *s) {
    u32 len = 0;
    while (s[len])
        len++;
    return len;
}

u32 digit_count(int num) {
    u32 count = 0;
    if (num == 0) return 1;
    while (num > 0) {
        count++;
        num = num / 10;
    }
    return count;
}

void itoa(int num, char *buf) {
    int cnt = digit_count(num), index = cnt - 1;
    char x;
    if (num == 0 && cnt == 1) {
        buf[0] = '0';
        buf[1] = '\0';
    } else {
        while (num != 0) {
            x = num % 10;
            buf[index] = x + '0';
            index--;
            num = num / 10;
        }
        buf[cnt] = '\0';
    }
}

const char *utoa(u32 num, char *buf, u8 radix) {
    u32 index = 0;
    char x;
    if (num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
    } else {
        while (num != 0) {
            x = num % radix;
            if (x < 10) buf[index] = x + '0';
            else buf[index] = x + 'A' - 10;
            index++;
            num = num / radix;
        }
        buf[index] = '\0';
    }
    return buf;
}

void cpuid(u32 value, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx) {
    u32 eaxres, ebxres, ecxres, edxres;

    asm("xorl %eax, %eax");
    asm("xorl %ebx, %ebx");
    asm("xorl %ecx, %ecx");
    asm("xorl %edx, %edx");
    asm("movl %0, %%eax" : "=m"(value));
    asm("cpuid");
    asm("movl %%eax, %0" : "=m"(eaxres));
    asm("movl %%ebx, %0" : "=m"(ebxres));
    asm("movl %%ecx, %0" : "=m"(ecxres));
    asm("movl %%edx, %0" : "=m"(edxres));

    *eax = eaxres;
    *ebx = ebxres;
    *ecx = ecxres;
    *edx = edxres;
}

u8 inb(u16 port) {
    u8 ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "d"(port));
    return ret;
}

void outb(u16 port, u8 data) {
    asm volatile("outb %0, %1" : "=a"(data) : "d"(port));
}

void wait_for_io(u32 timer_count) {
    for (;;) {
        asm volatile("nop");
        timer_count--;
        if (timer_count <= 0) break;
    }
}

void sleep(u32 timer_count) { wait_for_io(timer_count * 0x02ffffff); }