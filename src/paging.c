#include "include/paging.h"

void *paging_map_page(void *virt, void *phys, u16 flags) {}

void paging_unmap_page(void *virt) {}

void *paging_remap_page(void *old, void *new) {}

void *paging_walk_page(void *virt) {}

void *paging_edit_page(void *virt, u16 flags) {}

void paging_load_pml4(paging_table_t *pml4) {}

void *paging_get_pml4() {}