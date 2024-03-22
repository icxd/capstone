#include "include/pmm.h"
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