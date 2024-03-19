#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "types.h"

typedef struct {
    void *addr;
    char *name;
} symbol_t;

extern const WEAK symbol_t __symbol_tab[];

#endif // !SYMBOLS_H