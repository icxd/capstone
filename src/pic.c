#include "include/pic.h"
#include "include/kernel.h"
#include "include/types.h"

void pic_disable() {
    pic_remap(0x20);
    for (u8 irq = 0; irq < 16; irq++)
        pic_mask(irq);
}

void pic_remap(u8 offset) {
    u8 master_mask, slave_mask;

    master_mask = inb(PIC_MASTER_DATA);
    slave_mask = inb(PIC_SLAVE_DATA);

    outb(PIC_MASTER_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    outb(PIC_SLAVE_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);

    outb(PIC_MASTER_DATA, offset);
    outb(PIC_SLAVE_DATA, offset + 0x8);

    outb(PIC_MASTER_DATA, 0x4);
    outb(PIC_SLAVE_DATA, 0x2);

    outb(PIC_MASTER_DATA, PIC_ICW4_8086);
    outb(PIC_SLAVE_DATA, PIC_ICW4_8086);

    outb(PIC_MASTER_DATA, master_mask);
    outb(PIC_SLAVE_DATA, slave_mask);
}

void pic_mask(u8 irq) {
    u16 port;
    u8 masks;

    if (irq < 8) port = PIC_MASTER_DATA;
    else {
        port = PIC_SLAVE_DATA;
        irq -= 8;
    }

    masks = inb(port);
    masks |= (1 << irq);
    outb(port, masks);
}

void pic_unmask(u8 irq) {
    u16 port;
    u8 masks;

    if (irq < 8) port = PIC_MASTER_DATA;
    else {
        port = PIC_SLAVE_DATA;
        irq -= 8;
    }

    masks = inb(port);
    masks &= ~(1 << irq);
    outb(port, masks);
}

void pic_send_eoi(u8 irq) {
    if (irq >= 8) outb(PIC_SLAVE_COMMAND, PIC_EOI);
    outb(PIC_MASTER_COMMAND, PIC_EOI);
}