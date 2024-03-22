#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"

#define SERIAL_COM1_PORT 0x3f8

void s_init(void);
void s_printf(const char *, ...);
void s_puts(const char *);
void s_pute(const char *);
void s_set_input_masked(bool);

#endif // !SERIAL_H