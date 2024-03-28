#include "include/vmm.h"
#include "include/kernel.h"
#include "include/pmm.h"
#include "include/serial.h"
#include "include/types.h"

page_directory_t *kernel_page_dir;

bool vmm_alloc_page(void *vaddr) {
    void *paddr = pmm_alloc_block();
    if (!paddr) {
        return false;
    }
    vmm_map_page(paddr, vaddr);
    return true;
}

bool vmm_alloc_page_with_userbit(void *vaddr) {
    void *paddr = pmm_alloc_block();
    if (!paddr) {
        return false;
    }
    vmm_map_page(paddr, vaddr);
    page_table_entry_t *pte = (void *)GET_PTE((usz)vaddr);
    page_table_entry_add_attrib(pte, I86_PTE_USER);
    return true;
}

void vmm_free_page(void *vaddr) {
    page_table_entry_t *pte = (void *)GET_PTE((usz)vaddr);
    if (!page_table_entry_is_present(*pte)) {
        s_printf("oh, you try to delete not present page\n");
        // TODO: panic
        return;
    }
    void *block = page_table_entry_frame(*pte);
    if (block) {
        pmm_free_block(block);
    }
    page_table_entry_del_attrib(pte, I86_PTE_PRESENT);
}

void vmm_create_kernel_page_dir() {
    kernel_page_dir = (page_directory_t *)pmm_alloc_block();
    if (!kernel_page_dir) {
        s_printf("Failed to allocate phys memory for kernel page dir\n");
        // TODO: panic
        return;
    }

    page_directory_t *pd = kernel_page_dir;
    memset(pd, 0, sizeof(page_directory_t));

    for (usz i = 0; i < PAGE_ENTRIES; i++) {
        page_dir_entry_t *pde = (page_dir_entry_t *)&pd->entries[i];
        page_dir_entry_add_attrib(pde, I86_PTE_WRITABLE);
        page_dir_entry_del_attrib(pde, I86_PTE_PRESENT);

        if (i == PAGE_ENTRIES - 1) { // Fractal(recursive) mapping technique,
                                     // which allows us to access PD and PT
            page_dir_entry_add_attrib(pde, I86_PTE_PRESENT);
            page_dir_entry_set_frame(pde, kernel_page_dir);

            // s_printf("pd[1023] = 0x%x\n", pd->entries[1023]);
        }
    }
}

void vmm_map_page(void *paddr, void *vaddr) {
    page_dir_entry_t *pde = (void *)GET_PDE((usz)vaddr);
    if (!page_dir_entry_is_present(
            *pde)) {                    // If page table isnt present, create it
        void *pt_p = pmm_alloc_block(); // It's phys addr!
        if (!pt_p) {
            s_printf("wtf? no free phys memory\n");
            // TODO: panic
            return;
        }

        page_table_t *pt_v = (page_table_t *)vmm_temp_map_page(pt_p);
        memset(pt_v, 0, sizeof(page_table_t));
        page_dir_entry_add_attrib(pde, I86_PDE_PRESENT);
        page_dir_entry_add_attrib(pde, I86_PDE_WRITABLE);
        page_dir_entry_set_frame(pde, pt_p);
    }

    page_table_entry_t *pte = (void *)GET_PTE((usz)vaddr);
    page_table_entry_set_frame(pte, paddr);
    page_table_entry_add_attrib(pte, I86_PTE_PRESENT);
    page_table_entry_add_attrib(pte, I86_PTE_WRITABLE);
    flush_tlb_entry(vaddr);
}

void *vmm_temp_map_page(void *paddr) {
    page_table_entry_t *pte = (void *)GET_PTE((usz)TEMP_PAGE_ADDR);
    page_table_entry_set_frame(pte, (void *)PAGE_ALIGN_DOWN((usz)paddr));
    page_table_entry_add_attrib(pte, I86_PTE_PRESENT);
    page_table_entry_add_attrib(pte, I86_PTE_WRITABLE);

    // flush_tlb_entry(TEMP_PAGE_ADDR);
    asm volatile("invlpg %0" ::"m"(*(u32 *)TEMP_PAGE_ADDR) : "memory");
    // flush_tlb_all();

    return (void *)TEMP_PAGE_ADDR;
}

// Switch page directory, reveives physical address
void vmm_switch_page_directory(page_directory_t *page_dir_phys_addr) {
    asm volatile("mov %0, %%cr3" ::"r"((u32)page_dir_phys_addr));
}

void vmm_init() {
    // s_printf("1\n");

    vmm_create_kernel_page_dir();

    page_table_t *table1 = (page_table_t *)pmm_alloc_block();
    page_table_t *table2 = (page_table_t *)pmm_alloc_block();

    // Clear allocated page tables
    memset((void *)table1, 0, sizeof(page_table_t));
    memset((void *)table2, 0, sizeof(page_table_t));

    // Maps first MB to 3GB
    u8 *frame, *virt;
    for (frame = (u8 *)0x0, virt = (u8 *)0xC0000000;
         frame < (u8 *)0x100000 /*0x100000*/;
         frame += PAGE_SIZE, virt += PAGE_SIZE) {
        page_table_entry_t page = 0;
        page_table_entry_add_attrib(&page, I86_PTE_PRESENT);
        page_table_entry_set_frame(&page, frame);
        table1->entries[PAGE_TABLE_INDEX((usz)virt)] = page;
    }

    // Maps kernel pages and phys mem pages
    for (frame = (u8 *)KERNEL_START_PADDR, virt = (u8 *)KERNEL_START_VADDR;
         frame < (u8 *)KERNEL_PHYS_MAP_END;
         frame += PAGE_SIZE, virt += PAGE_SIZE) {
        page_table_entry_t page = 0;
        page_table_entry_add_attrib(&page, I86_PTE_PRESENT);
        page_table_entry_set_frame(&page, frame);
        table2->entries[PAGE_TABLE_INDEX((usz)virt)] = page;
    }

    page_dir_entry_t *pde1 =
        &kernel_page_dir->entries[PAGE_DIRECTORY_INDEX(0x00000000)];
    page_dir_entry_add_attrib(pde1, I86_PDE_PRESENT);
    page_dir_entry_add_attrib(pde1, I86_PDE_WRITABLE);
    page_dir_entry_set_frame(pde1, table1);

    page_dir_entry_t *pde2 =
        &kernel_page_dir->entries[PAGE_DIRECTORY_INDEX(0xC0100000)];
    page_dir_entry_add_attrib(pde2, I86_PDE_PRESENT);
    page_dir_entry_add_attrib(pde2, I86_PDE_WRITABLE);
    page_dir_entry_set_frame(pde2, table2);

    pmm_update_bitmap_addr((void *)KERNEL_END_VADDR);

    enable_paging(kernel_page_dir);

    s_printf("Virtual memory manager initialized!\n");
}

void vmm_test() {
    s_printf("kernel_page_dir = 0x%x\n", kernel_page_dir);

    void *padr1 = (void *)0xC0500000;
    void *vadr1 = vmm_temp_map_page(padr1);
    *(u8 *)vadr1 = 77;
    s_printf("0x%x = 0x%x\n", padr1, *(u8 *)vadr1);

    s_printf("0x%x = 0x%x\n", (0x00100000), *(u8 *)(0x00100000)); // IT
    // WILL CAUSE PAGE FAULT!!!! BEACUSE WE 1:1 MAPPED UP TO 1MB PHYS MEM BUT
    // NEVKLYUCHITELNO!
    s_printf("0x%x = 0x%x\n", (0x00100000 - 1), *(u8 *)(0x00100000 - 1));
    // asm volatile( "movl %0, %%cr3" :: "r" (kernel_page_dir));

    int eip;
    asm volatile("1: lea 1b, %0" : "=a"(eip));
    s_printf("EIP = 0x%x  ", eip);
}

// Add attribute to pte
void page_table_entry_add_attrib(page_table_entry_t *entry, u32 attrib) {
    *entry |= attrib;
}

// Delete attribute to pte
void page_table_entry_del_attrib(page_table_entry_t *entry, u32 attrib) {
    *entry &= ~attrib;
}

// Map pte to physical frame
void page_table_entry_set_frame(page_table_entry_t *entry, void *addr) {
    *entry = (*entry & ~I86_PTE_FRAME) | (usz)addr;
}

bool page_table_entry_is_present(page_table_entry_t entry) {
    return entry & I86_PTE_PRESENT;
}

bool page_table_entry_is_writable(page_table_entry_t entry) {
    return entry & I86_PTE_WRITABLE;
}

// Return the address of physical frame which pte refers to
void *page_table_entry_frame(page_table_entry_t entry) {
    return (void *)(entry & I86_PTE_FRAME);
}

// Functions for Page Directory Entries

// Add attribute to pde
void page_dir_entry_add_attrib(page_dir_entry_t *entry, u32 attrib) {
    *entry |= attrib;
}

// Delete attribute to pde
void page_dir_entry_del_attrib(page_dir_entry_t *entry, u32 attrib) {
    *entry &= ~attrib; // old: was without ~ !!
}

// Map pde to physical frame (where the appropriate page table stores)
void page_dir_entry_set_frame(page_dir_entry_t *entry, void *addr) {
    *entry = (*entry & ~I86_PDE_FRAME) | (usz)addr;
}

bool page_dir_entry_is_present(page_dir_entry_t entry) {
    return entry & I86_PDE_PRESENT;
}

bool page_dir_entry_is_user(page_dir_entry_t entry) {
    return entry & I86_PDE_USER;
}

bool page_dir_entry_is_4mb(page_dir_entry_t entry) {
    return entry & I86_PDE_4MB;
}

bool page_dir_entry_is_writable(page_dir_entry_t entry) {
    return entry & I86_PDE_WRITABLE;
}

// Return the address of physical frame which pde refers to
void *page_dir_entry_frame(page_dir_entry_t entry) {
    return (void *)(entry & I86_PDE_FRAME);
}

void flush_tlb_entry(void *addr) {
    asm volatile("invlpg (%0)" : : "b"(addr) : "memory");
}
