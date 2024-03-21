#ifndef VGA_H
#define VGA_H

#include "types.h"

#define VGA_ADDR 0xb8000
#define BUFSIZE 2200

typedef enum {
    VGA_BLACK,
    VGA_BLUE,
    VGA_GREEN,
    VGA_CYAN,
    VGA_RED,
    VGA_MAGENTA,
    VGA_BROWN,
    VGA_GREY,
    VGA_DARK_GREY,
    VGA_BRIGHT_BLUE,
    VGA_BRIGHT_GREEN,
    VGA_BRIGHT_CYAN,
    VGA_BRIGHT_RED,
    VGA_BRIGHT_MAGENTA,
    VGA_YELLOW,
    VGA_WHITE,
} vga_color_t;

static u16 *vga_buffer;
static u32 vga_index, next_line_index = 1;
static u8 g_fore_color = VGA_YELLOW, g_back_color = VGA_BLACK;
static int digit_ascii_codes[10] = {0x30, 0x31, 0x32, 0x33, 0x34,
                                    0x35, 0x36, 0x37, 0x38, 0x39};

u16 v_entry(u8, u8, u8);
void v_clear_buffer(u16 **, u8, u8);
void v_init(u8, u8);
void v_putnl();
void v_printf(const char *, ...);
void v_putc(char);
void v_puts(char *);
void v_puti(int);
void v_putb(u32);

#endif // !VGA_H