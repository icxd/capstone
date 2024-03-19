#include "include/serial.h"
#include "include/kernel.h"

void serial_init() {
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

static void serial_write_char(char *ch) {
    while ((inb(SERIAL_COM1_PORT + 5) & 0x20) == 0)
        ;
    outb(SERIAL_COM1_PORT, ch);
}

void serial_write(const char *s) {
    for (int i = 0; s[i] != '\0'; i++)
        serial_write_char(s[i]);
}

void serial_write_error(const char *s) {
    serial_write("[ERROR] ");
    serial_write(s);
}

void serial_set_input_masked(bool);
