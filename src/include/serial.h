#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"

#define SERIAL_COM1_PORT 0x3f8

void serial_init(void);
void serial_write(const char *);
void serial_write_error(const char *);
void serial_set_input_masked(bool);

#endif // !SERIAL_H