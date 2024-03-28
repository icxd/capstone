#ifndef VMM_H
#define VMM_H

#include "memlayout.h"
#include "types.h"

typedef u32 page_table_entry_t;
typedef u32 page_dir_entry_t;

typedef struct ALIGNED(4096) page_table_t {
    page_table_entry_t entries[PAGES_PER_TABLE];
} page_table_t;

typedef struct ALIGNED(4096) page_directory_t {
    page_dir_entry_t entries[PAGES_PER_DIR];
} page_directory_t;

enum PAGE_PTE_FLAGS {
    I86_PTE_PRESENT = 1,
    I86_PTE_WRITABLE = 2,
    I86_PTE_USER = 4,
    I86_PTE_WRITETHOUGH = 8,
    I86_PTE_NOT_CACHEABLE = 0x10,
    I86_PTE_ACCESSED = 0x20,
    I86_PTE_DIRTY = 0x40,
    I86_PTE_PAT = 0x80,
    I86_PTE_CPU_GLOBAL = 0x100,
    I86_PTE_LV4_GLOBAL = 0x200,
    I86_PTE_FRAME = 0x7FFFF000,
};

enum PAGE_PDE_FLAGS {
    I86_PDE_PRESENT = 1,
    I86_PDE_WRITABLE = 2,
    I86_PDE_USER = 4,
    I86_PDE_PWT = 8,
    I86_PDE_PCD = 0x10,
    I86_PDE_ACCESSED = 0x20,
    I86_PDE_DIRTY = 0x40,
    I86_PDE_4MB = 0x80,
    I86_PDE_CPU_GLOBAL = 0x100,
    I86_PDE_LV4_GLOBAL = 0x200,
    I86_PDE_FRAME = 0x7FFFF000
};

#define PAGE_ALIGN_DOWN(x) ((x) & -PAGE_SIZE)
#define PAGE_ALIGN_UP(address) (((address) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3FF)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3FF)

#define PAGE_GET_TABLE_ADDRESS(x) (*x & ~0xFFF)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xFFF)

#define GET_PDE(v) (page_dir_entry_t *)(0xFFFFF000 + (v >> 22) * 4)
#define GET_PTE(v) (page_table_entry_t *)(0xFFC00000 + (v >> 12) * 4)

extern page_directory_t *kernel_page_dir;

void page_table_entry_add_attrib(page_table_entry_t *, u32);
void page_table_entry_del_attrib(page_table_entry_t *, u32);
void page_table_entry_set_frame(page_table_entry_t *, void *);
bool page_table_entry_is_present(page_table_entry_t);
bool page_table_entry_is_writable(page_table_entry_t);
void *page_table_entry_frame(page_table_entry_t);

void page_dir_entry_add_attrib(page_dir_entry_t *, u32);
void page_dir_entry_del_attrib(page_dir_entry_t *, u32);
void page_dir_entry_set_frame(page_dir_entry_t *, void *);
bool page_dir_entry_is_present(page_dir_entry_t);
bool page_dir_entry_is_user(page_dir_entry_t);
bool page_dir_entry_is_4mb(page_dir_entry_t);
bool page_dir_entry_is_writable(page_dir_entry_t);
void *page_dir_entry_frame(page_dir_entry_t);

void flush_tlb_entry(void *);

extern void enable_paging(void *);

void vmm_init();
void vmm_create_kernel_page_dir();

bool vmm_alloc_page(void *);
bool vmm_alloc_page_with_userbit(void *);
void vmm_free_page(void *);
void vmm_map_page(void *, void *);
void *vmm_temp_map_page(void *);
void vmm_switch_page_directory(page_directory_t *);

void vmm_test();

#endif // VMM_H