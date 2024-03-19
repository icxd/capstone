#include "include/vga.h"
#include "include/kernel.h"

u16 v_entry(u8 ch, u8 fg, u8 bg) {
    u16 ax = 0;
    u8 ah = 0, al = 0;

    ah = bg;
    ah <<= 4;
    ah |= fg;
    ax = ah;
    ax <<= 8;
    al = ch;
    ax |= al;

    return ax;
}

void v_clear_buffer(u16 **buf, u8 fg, u8 bg) {
    u32 i;
    for (i = 0; i < BUFSIZE; i++)
        (*buf)[i] = v_entry(null, fg, bg);
    next_line_index = 1;
    vga_index = 0;
}

void v_init(u8 fg, u8 bg) {
    vga_buffer = (u16 *)VGA_ADDR;
    v_clear_buffer(&vga_buffer, fg, bg);
    g_fore_color = fg;
    g_back_color = bg;
}

void v_putnl() {
    if (next_line_index >= 55) {
        next_line_index = 0;
        v_clear_buffer(&vga_buffer, g_fore_color, g_back_color);
    }

    vga_index = 80 * next_line_index;
    next_line_index++;
}

void v_putc(char ch) {
    if (ch == '\n') v_putnl();
    else if (ch == '\t') {
        vga_buffer[vga_index++] = v_entry(9, g_back_color, g_back_color);
        vga_buffer[vga_index++] = v_entry(9, g_back_color, g_back_color);
    } else {
        vga_buffer[vga_index++] = v_entry(ch, g_fore_color, g_back_color);
    }
}

void v_puts(char *s) {
    u32 i = 0;
    while (s[i])
        v_putc(s[i++]);
}

void v_puti(int num) {
    char buf[digit_count(num) + 1];
    itoa(num, buf);
    v_puts(buf);
}

void v_putb(u32 num) {
    char buf[32];
    u32 idx = 31, i;
    while (num > 0) {
        if (num & 1) buf[idx] = '1';
        else buf[idx] = '0';
        idx--;
        num >>= 1;
    }

    for (i = 0; i < 32; i++) {
        if (i <= idx) v_putc('0');
        else v_putc(buf[i]);
    }
}