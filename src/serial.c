#include "include/serial.h"
#include "include/kernel.h"

void s_init() {
    u16 port = SERIAL_COM1_PORT;

    outb(port + 1, 0x00);
    outb(port + 3, 0x80);
    outb(port + 0, 0x0c);
    outb(port + 1, 0x00);
    outb(port + 3, 0x03);
    outb(port + 2, 0xc7);
    outb(port + 4, 0x0b);
    outb(port + 0, 0xae);

    if (inb(port) != 0xae) return;

    outb(port + 4, 0x0f);
}

static void s_putc(char ch) {
    while ((inb(SERIAL_COM1_PORT + 5) & 0x20) == 0)
        ;
    outb(SERIAL_COM1_PORT, ch);
}

void s_printf(const char *fmt, ...) {
    __builtin_va_list ap;
    __builtin_va_start(ap, fmt);

    u32 i = 0;
    while (fmt[i]) {
        if (fmt[i] == '%') {
            i++;
            switch (fmt[i]) {
            case 'c':
                s_putc(__builtin_va_arg(ap, int));
                break;

            case 's':
                s_puts(__builtin_va_arg(ap, char *));
                break;

            case 'd': {
                char utoa_buf[32];
                utoa(__builtin_va_arg(ap, u32), utoa_buf, 10);
                s_puts(utoa_buf);
            } break;

            case 'b': {
                char utoa_buf[32];
                utoa(__builtin_va_arg(ap, u32), utoa_buf, 2);
                s_puts(utoa_buf);
            } break;

            case 'x': {
                char utoa_buf[32];
                utoa(__builtin_va_arg(ap, u32), utoa_buf, 16);
                s_puts(utoa_buf);
            } break;

            case 'p': {
                char utoa_buf[32];
                utoa(__builtin_va_arg(ap, u32), utoa_buf, 16);
                s_puts("Ox");
                s_puts(utoa_buf);
            } break;

            default:
                break;
            }
        } else {
            v_putc(fmt[i]);
        }
        i++;
    }

    __builtin_va_end(ap);
}

void s_puts(const char *s) {
    for (int i = 0; s[i] != '\0'; i++)
        s_putc(s[i]);
}

void s_pute(const char *s) {
    s_puts("[ERROR] ");
    s_puts(s);
}

void s_set_input_masked(bool) {}
