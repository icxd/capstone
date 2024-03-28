#ifndef KHEAP_H
#define KHEAP_H

#include "types.h"

typedef struct kheap_item_t {
    struct kheap_item_t *next;
    usz size;
    bool used;
} PACKED kheap_item_t;

extern kheap_item_t *kheap_begin, *kheap_end;
extern u8 *kheap_limit;
extern usz kheap_memory_used, kheap_allocs_num;

void kheap_init(void);
usz kheap_increase(usz);
void kheap_merge_free_adjacent(void);
void kfree(void *);
void *kmalloc(usz);
void *krealloc(void *, usz);
void kheap_print_stat(void);
void kheap_test(void);

#endif // KHEAP_H