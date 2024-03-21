#ifndef PAGING_H
#define PAGING_H

#include "types.h"

#define PAGING_FLAG_PRESENT 0x001
#define PAGING_FLAG_WRITE 0x002
#define PAGING_FLAG_USER 0x004
#define PAGING_FLAG_WRITE_THROUGH 0x008
#define PAGING_FLAG_CACHE_DISABLE 0x010
#define PAGING_FLAG_ACCESSED 0x020
#define PAGING_FLAG_LARGER_PAGES 0x040
#define PAGING_FLAG_OS_AVAILABLE 0xe00
#define PAGING_FLAG_NO_EXECUTE (1 << 63)

#define PAGING_FLAGS_KERNEL_PAGE (PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE)
#define PAGING_FLAGS_USER_PAGE                                                 \
    (PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE | PAGING_FLAG_USER)

#define PAGING_PAGE_SIZE 0x1000
#define PAGING_PAGE_BOUNDARY 0x1000
#define PAGING_PAGE_ALIGNED ALIGNED(PAGING_PAGE_SIZE)

#define PAGING_KERNEL_OFFSET 0xffffffff80000000
#define PAGING_VIRTUAL_OFFSET 0xffff000000000000

#define nullvptr -PAGING_VIRTUAL_OFFSET == null

typedef u64 paging_desc_t;

typedef struct {
    u16 pml4, pml3, pml2, pml1;
} paging_indexer_t;

typedef struct {
    paging_desc_t entries[512];
} PAGING_PAGE_ALIGNED paging_table_t;

static ALWAYS_INLINE inline void
paging_desc_set_address(paging_desc_t *descriptor, u64 address) {
    *descriptor |= (u64)address & 0xffffffffff000;
}

static ALWAYS_INLINE inline void
paging_indexer_assign(paging_indexer_t *indexer, void *address) {
    u64 uaddress = (u64)address;
    uaddress >>= 12;
    indexer->pml1 = uaddress & 0x1ff;
    uaddress >>= 9;
    indexer->pml2 = uaddress & 0x1ff;
    uaddress >>= 9;
    indexer->pml3 = uaddress & 0x1ff;
    uaddress >>= 9;
    indexer->pml4 = uaddress & 0x1ff;
}

static ALWAYS_INLINE inline void *
paging_desc_get_address(paging_desc_t *descriptor) {
    return (void *)(*descriptor & 0xffffffffff000);
}

static __attribute__((always_inline)) inline bool
paging_desc_get_flag(paging_desc_t *descriptor, u64 flag) {
    return *descriptor & flag;
}

static __attribute__((always_inline)) inline void
paging_desc_set_flag(paging_desc_t *descriptor, u64 flag, bool value) {
    *descriptor &= ~flag;
    if (value) *descriptor |= flag;
}

static __attribute__((always_inline)) inline void
paging_desc_set_flags(paging_desc_t *descriptor, u64 flags) {
    *descriptor &= ~0x0ffful;
    *descriptor |= flags;
}

static __attribute__((always_inline)) inline void paging_invlpg(void *page) {
    asm volatile("invlpg (%0)" : : "r"(page) : "memory");
}

void *paging_map_page(void *, void *, u16);
void paging_unmap_page(void *);
void *paging_remap_page(void *, void *);
void *paging_walk_page(void *);
void *paging_edit_page(void *, u16);
void paging_load_pml4(paging_table_t *);
void *paging_get_pml4();

#endif // !PAGING_H