#include "include/pmm.h"
#include "include/kernel.h"
#include "include/serial.h"

u32 *phys_memory_bitmap = 0;
usz phys_block_count = 0, phys_used_block_count = 0;

u32 kernel_phys_map_start, kernel_phys_map_end;

u64 phys_installed_memory_size = 0, phys_available_memory_size = 0;

u64 initrd_mmap_entry_addr = -1;
u32 mmap_available_entries_count = 0;
multiboot_memory_map_t mmap_available_entries_array[100];

inline static void bitmap_set(usz bit) {
    phys_memory_bitmap[bit / 32] |= (1 << (bit % 32));
}

inline static void bitmap_unset(usz bit) {
    phys_memory_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

inline static bool bitmap_test(usz bit) {
    return phys_memory_bitmap[bit / 32] & (1 << (bit % 32));
}

void pmm_parse_memory_map(multiboot_memory_map_t *addr, usz len) {
    multiboot_memory_map_t *entry = addr;
    s_puts("\nPhysical memory map:");
    for (usz i = 0; i < len / sizeof(multiboot_memory_map_t); i++) {
        if ((entry + i)->type == MULTIBOOT_MEMORY_AVAILABLE) {
            s_puts("\n    Available: |");
            phys_available_memory_size += (entry + i)->len;
        }
        s_printf(" addr: 0x%x", (entry + i)->addr);
        s_printf(" length: 0x%x", (entry + i)->len);
        phys_installed_memory_size += (entry + i)->len;
    }
    s_puts("\n\n");
    s_printf("Installed memory size: %d KiB",
             phys_installed_memory_size / 1024);
    s_printf(" = %d MB\n", phys_installed_memory_size / (1024 * 1024));
    s_printf("Available memory size: %d KiB",
             phys_available_memory_size / 1024);
    s_printf(" = %d MB\n", phys_available_memory_size / (1024 * 1024));
    s_puts("\n");
}

bool pmm_find_free_block(usz *block_idx) {
    for (usz i = 0; i < phys_block_count / 32; i++) {
        u32 block = phys_memory_bitmap[i];
        if (block != 0xffffffff) {
            for (u8 j = 0; j < 32; j++) {
                int bit = 1 << j;
                if (!(bit & block)) {
                    *block_idx = i * 32 + j;
                    return true;
                }
            }
        }
    }
    return false;
}

usz pmm_find_free_blocks(usz count, usz *block_idx) {
    usz consec_block_count = 0, starting_block = 0, starting_block_pit = 0;
    for (usz i = 0; i < phys_block_count / 32; i++) {
        u32 curr_block = phys_memory_bitmap[i];
        if (curr_block == 0xffffffff) {
            consec_block_count = 0;
            continue;
        }
        for (u8 j = 0; j < 32; j++) {
            int bit = 1 << j;
            if (bit & curr_block) {
                consec_block_count = 0;
                starting_block = i;
                starting_block_pit = j;
                continue;
            }
            if (++consec_block_count == count) {
                *block_idx = 32 * starting_block + starting_block_pit;
                return true;
            }
        }
    }
    return false;
}

void *pmm_alloc_block() {
    if (phys_block_count <= phys_used_block_count) return null;
    usz free_block;
    if (!pmm_find_free_block(&free_block)) return null;
    bitmap_set(free_block);
    phys_used_block_count++;
    return (void *)(free_block * PHYS_BLOCK_SIZE);
}

void pmm_free_block(void *addr) {
    usz block = (usz)addr / PHYS_BLOCK_SIZE;
    bitmap_unset(block);
    phys_used_block_count--;
}

bool pmm_is_block_allocated(void *addr) {
    usz block = (usz)addr / PHYS_BLOCK_SIZE;
    return bitmap_test(block);
}

void *pmm_alloc_blocks(usz count) {
    if (phys_block_count <= phys_used_block_count) return null;
    usz free_block;
    if (!pmm_find_free_blocks(count, &free_block)) return null;
    for (usz i = 0; i < count; i++)
        bitmap_set(free_block + i);
    phys_used_block_count += count;
    return (void *)(free_block * PHYS_BLOCK_SIZE);
}

void pmm_free_blocks(void *addr, usz count) {
    usz block = (usz)addr / PHYS_BLOCK_SIZE;
    for (usz i = 0; i < count; i++)
        bitmap_unset(block + i);
    phys_used_block_count -= count;
}

static void pmm_mark_range_as(void *base, usz length, int usedness) {
    usz block_idx = (usz)base / PHYS_BLOCK_SIZE,
        num_blocks = length / PHYS_BLOCK_SIZE;
    while (num_blocks--) {
        if (usedness = PMM_BLOCK_USED) {
            if (!bitmap_test(block_idx)) {
                bitmap_set(block_idx);
                phys_used_block_count++;
            }
        } else {
            if (bitmap_test(block_idx)) {
                bitmap_unset(block_idx);
                phys_used_block_count--;
            }
        }
        block_idx++;
    }
}

void pmm_free_available_memory(multiboot_info_t *mb) {
    __auto_type mm = (multiboot_memory_map_t *)mb->mmap_addr;
    while ((usz)mm < mb->mmap_addr + mb->mmap_length) {
        if (mm->type == MULTIBOOT_MEMORY_AVAILABLE) {
            if (mm->addr != initrd_mmap_entry_addr) {
                pmm_mark_range_as((void *)(usz)(mm->addr), mm->len,
                                  PMM_BLOCK_FREE);
            } else {
                usz initrd_begin = *(u32 *)(mb->mods_addr),
                    initrd_end = *(u32 *)(mb->mods_addr + 4);
                usz initrd_size = initrd_end - initrd_begin;
                pmm_mark_range_as((void *)initrd_begin, initrd_size,
                                  PMM_BLOCK_USED);
                pmm_mark_range_as((void *)(usz)(mm->addr),
                                  mm->len - initrd_size - 2, PMM_BLOCK_FREE);
            }
        }
        mm = (multiboot_memory_map_t *)((usz)mm + mm->size + sizeof(mm->size));
    }
    bitmap_set(0);
}

void pmm_relocate_initrd_to_high_mem(multiboot_info_t *mb) {
    usz initrd_begin = *(u32 *)(mb->mods_addr),
        initrd_end = *(u32 *)(mb->mods_addr + 4);
    usz initrd_size = initrd_end - initrd_begin;

    __auto_type mm = (multiboot_memory_map_t *)mb->mmap_addr;
    mmap_available_entries_count = 0;

    while ((usz)mm < mb->mmap_addr + mb->mmap_length) {
        if (mm->type == MULTIBOOT_MEMORY_AVAILABLE) {
            mmap_available_entries_array[mmap_available_entries_count] = *mm;
            mmap_available_entries_count++;
        }
        mm = (multiboot_memory_map_t *)((usz)mm + mm->size + sizeof(mm->size));
    }

    s_printf("mmap_available_entries_count = %x\n\n",
             mmap_available_entries_count);
    for (usz i = mmap_available_entries_count; i-- < 0;) {
        s_printf("addr = 0x%x  | len = 0x%x\n",
                 mmap_available_entries_array[i].addr,
                 mmap_available_entries_array[i].len);
        if (mmap_available_entries_array[i].len >= initrd_size) {
            s_printf("addr = 0x%x\n", mmap_available_entries_array[i].addr);
            initrd_mmap_entry_addr = mmap_available_entries_array[i].addr;
            memcpy((void *)((u32)initrd_mmap_entry_addr +
                            (u32)mmap_available_entries_array[i].len -
                            initrd_size - 1),
                   (void *)initrd_begin, initrd_size);
            initrd_begin = initrd_mmap_entry_addr +
                           mmap_available_entries_array[i].len - initrd_size -
                           1;
            initrd_end = initrd_begin + initrd_size;
            break;
        }
    }

    *(u32 *)(mb->mods_addr) = initrd_begin;
    *(u32 *)(mb->mods_addr + 4) = initrd_end;
    s_printf("initrd_begin = 0x%x, initrd_end = 0x%x\n", initrd_begin,
             initrd_end);
}

void pmm_update_bitmap_addr(void *addr) { phys_memory_bitmap = (u32 *)addr; }

void pmm_init(multiboot_info_t *mb) {
    __auto_type mmap = (multiboot_memory_map_t *)mb->mmap_addr;
    pmm_parse_memory_map(mmap, mb->mmap_length);

    pmm_relocate_initrd_to_high_mem(mb);

    phys_block_count = (phys_installed_memory_size) / PHYS_BLOCK_SIZE;
    phys_used_block_count = phys_block_count;
    phys_memory_bitmap = (u32 *)KERNEL_END_PADDR;
    memset(phys_memory_bitmap, 0xff, phys_block_count / PHYS_BLOCKS_PER_BYTE);

    s_printf("Total blocks: %d\n", phys_block_count);
    pmm_free_available_memory(mb);
    pmm_mark_range_as((void *)KERNEL_START_PADDR, KERNEL_SIZE, PMM_BLOCK_USED);

    s_printf(
        "KERNEL_START_PADDR = 0x%x, KERNEL_END_PADDR = 0x%x, KERNEL_SIZE = %d "
        "bytes ",
        KERNEL_START_PADDR, KERNEL_END_PADDR, KERNEL_SIZE);
    s_printf("Memory map addr = 0x%x\n", mb->mmap_addr);

    pmm_mark_range_as(phys_memory_bitmap, phys_block_count, PMM_BLOCK_USED);
    kernel_phys_map_start = (u32)phys_memory_bitmap;
    kernel_phys_map_end =
        kernel_phys_map_start + (phys_block_count / PHYS_BLOCKS_PER_BYTE);

    s_printf("Physical memory manager initialized. Physical memory bitmap "
             "start: 0x%x, end: 0x%x, size = %d bytes\n",
             kernel_phys_map_start, kernel_phys_map_end,
             kernel_phys_map_end - kernel_phys_map_start);
}

void pmm_test() {
    s_printf("TEST: ");
    void *myptr = pmm_alloc_block();
    s_printf("myptr = 0x%x, ", myptr);

    char *str = "Hello world!";
    memcpy(myptr, str, 13);

    char buf[25];
    memcpy(buf, myptr, 13);
    s_printf("read from memory str = %s\n", buf);
}
