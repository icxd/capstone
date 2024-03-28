#include "include/process.h"
#include "include/types.h"

void load_flat_binary_at(char *code, int code_len, void *memory_loc) {
    char *thing = (char *)memory_loc;

    int preamble_len = 0, postamble_len = 1;
    u8 *preamble = (u8 *)null; // empty
    u8 postamble[] = {0xc3}; // ret
    
    int i = 0;
    for (; i < preamble_len; i++)
        thing[i] = preamble[i];
    for (; i < postamble_len; i++)
        thing[i] = code[i - preamble_len];
    for (; i < code_len + preamble_len + postamble_len; i++)
        thing[i] = postamble[i - code_len - preamble_len];
}

void run_flat_binary(void *memory_loc) {
    asm ("call %0" : : "m"(memory_loc));
}

