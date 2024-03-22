#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

#include "types.h"

#define KERNEL_START_PADDR ((u32) & kernel_phys_start)
#define KERNEL_END_PADDR ((u32) & kernel_phys_end)
#define KERNEL_SIZE (KERNEL_END_PADDR - KERNEL_START_PADDR)

#define KERNEL_START_VADDR ((u32) & kernel_virt_start)
#define KERNEL_END_VADDR ((u32) & kernel_virt_end)

#define KERNEL_PHYS_MAP_START (kernel_phys_map_start)
#define KERNEL_PHYS_MAP_END (kernel_phys_map_end)
#define KERNEL_PHYS_MAP_SIZE (kernel_phys_map_end - kernel_phys_map_start)

#define PHYS_BLOCKS_PER_BYTE 8
#define PHYS_BLOCK_SIZE 4096

#define TEMP_PAGE_ADDR 0xc03ff000
#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR 1024
#define PAGE_SIZE 4096
#define PAGE_ENTRIES 1024

#define KHEAP_START_VADDR 0xc0500000

#define ALIGN_BLOCK(addr) ((addr) - ((addr) % PHYS_BLOCK_SIZE))

extern u32 kernel_phys_start, kernel_phys_end;
extern u32 kernel_virt_start, kernel_virt_end;
// extern u32 kernel_phys_map_start, kernel_phys_map_end;

#endif // !MEMLAYOUT_H