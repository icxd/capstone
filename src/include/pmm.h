#ifndef PMM_H
#define PMM_H

#include "memlayout.h"
#include "multiboot.h"
#include "types.h"

#define PMM_BLOCK_FREE 0
#define PMM_BLOCK_USED 1

void pmm_parse_memory_map(multiboot_memory_map_t *, usz);
bool pmm_find_free_block(usz *);
usz pmm_find_free_blocks(usz, usz *);
void *pmm_alloc_block();
void pmm_free_block(void *);
bool pmm_is_block_allocated(void *);
void *pmm_alloc_blocks(usz);
void pmm_free_blocks(void *, usz);
void pmm_free_available_memory(multiboot_info_t *);
void pmm_relocate_initrd_to_high_mem(multiboot_info_t *);
void pmm_update_bitmap_addr(void *);
void pmm_init(multiboot_info_t *);

#endif // !PMM_H
