#include "include/kheap.h"
#include "include/kernel.h"
#include "include/memlayout.h"
#include "include/serial.h"
#include "include/vmm.h"

kheap_item_t *kheap_begin, *kheap_end;
usz kheap_memory_used, kheap_allocs_num;

void kheap_init(void) {
    kheap_begin = (kheap_item_t *)KHEAP_START_VADDR;
    kheap_end = null;

    kheap_allocs_num = 0;
    kheap_memory_used = 0;
}

usz kheap_increase(usz size) {
    // if (kheap_end) {
    //     s_printf("\nkheap_end = %p, size = %u, kheap_limit =  %p\n",
    //     kheap_end,
    //              size, kheap_limit);
    //     assert((u8 *)kheap_end + size < kheap_limit);
    // }
    usz pages = (size / PAGE_SIZE) + (size % PAGE_SIZE != 0);
    if (kheap_end == null) kheap_end = kheap_begin;

    s_printf("pages = %u\n", pages);
    for (usz i = 0; i < pages; i++) {
        vmm_alloc_page(kheap_end);
        memset((void *)kheap_end, 0, PAGE_SIZE);
        kheap_end = (kheap_item_t *)((usz)kheap_end + PAGE_SIZE);
    }

    return pages * PAGE_SIZE;
}

void kheap_merge_free_adjacent(void) {
    for (kheap_item_t *item = (kheap_item_t *)kheap_begin; item != null;
         item = item->next) {
        while (!item->used && item->next != null && !item->next->used) {
            item->size += sizeof(kheap_item_t) + item->next->size;
            item->next = item->next->next;
        }
    }
}

void kfree(void *addr) {
    s_printf("kfree(0x%x)\n", addr);
    if (addr == null) return;
    for (kheap_item_t *item = kheap_begin; item != null; item = item->next) {
        if (item == (kheap_item_t *)addr - 1) {
            item->used = 0;
            kheap_memory_used -= item->size;
            kheap_allocs_num--;
            kheap_merge_free_adjacent();
            break;
        }
    }
}

void *kmalloc(usz size) {
    s_printf("kmalloc(%u)\n", size);
    if (size == 0) return null;
    kheap_item_t *new_item = null, *tmp_item;
    usz total_size = size + sizeof(kheap_item_t);
    s_printf("total_size = %u\n", total_size);

    kheap_item_t *last_item = kheap_begin;
    if (kheap_end != null) {
        for (new_item = kheap_begin; new_item != null;
             new_item = new_item->next) {
            if (new_item->next == null) last_item = new_item;
            if (!new_item->used && (total_size <= new_item->size)) break;
        }
    }

    if (new_item != null) {
        if (new_item->size - total_size > 0) {
            tmp_item = (kheap_item_t *)((usz)new_item + total_size);
            tmp_item->size = new_item->size - total_size;
            tmp_item->used = 0;
            tmp_item->next = new_item->next;
            new_item->next = tmp_item;
        }
    } else {
        new_item = (kheap_end == null) ? kheap_begin : kheap_end;
        s_printf(">> new_item = %p\n", new_item);
        usz bytes_added = kheap_increase(total_size);

        s_printf("bytes_added = %u, total_size = %u\n", bytes_added,
                 total_size);
        if (bytes_added - total_size > sizeof(kheap_item_t)) {
            tmp_item = (kheap_item_t *)((usz)new_item + total_size);
            tmp_item->size = bytes_added - total_size - sizeof(kheap_item_t);
            tmp_item->used = 0;
            tmp_item->next = null;
            new_item->next = tmp_item;
        } else {
            new_item->next = null;
        }
        if (new_item != kheap_begin) {
            last_item->next = new_item;
        }
    }
    new_item->size = size;
    new_item->used = 1;

    kheap_allocs_num++;
    kheap_memory_used += total_size;

    s_printf("kmalloc_end\n");
    return (void *)(new_item + 1);
}

void *krealloc(void *p, usz newsz) {
    s_printf("krealloc(0x%x, %u)\n", p, newsz);
    void *p2 = kmalloc(newsz);
    memcpy(p2, p, *((u8 *)p - 1 - 4));
    kfree(p);
    return p2;
}

void kheap_print_stat(void) {
    s_printf("\nallocs number = %u", kheap_allocs_num);
    s_printf("\nmemory used = %u bytes\n", kheap_memory_used);
    for (kheap_item_t *item = kheap_begin; item != null; item = item->next)
        s_printf("  size: %u, used: %u\n", item->size, item->used);
    s_printf("\n");
}

void kheap_test(void) {
    u32 sz = 4096 - sizeof(kheap_item_t); // 1024 * 768 * 4;
    u8 *mas = (u8 *)kmalloc(sz);

    memset(mas, 5, sz);
    s_printf("mas[sz-1] = %d\n", mas[sz - 1]);
    s_printf("mas_addr = %p\n", mas);

    kheap_print_stat();

    int cnt = 12;
    int *arr = (int *)kmalloc(cnt * sizeof(int));

    int i;
    for (i = 0; i < cnt; i++)
        arr[i] = i * 2;

    // for (i = 0; i < cnt; i++) {
    //     s_printf("%d ", arr[i]);
    // }

    s_printf("arr = %p", arr);
    kheap_print_stat();

    // heap_free(arr);
    // s_printf("\narr[0] = %d ", arr[1]);
    // heap_print_stat();

    int *arr2 = (int *)kmalloc(24 * sizeof(int));
    for (i = 0; i < 24; i++)
        arr2[i] = i * 3;

    s_printf("\n");

    for (i = 0; i < 24; i++)
        s_printf("%d ", arr2[i]);

    s_printf("arr2 = %p", arr2);
    kheap_print_stat();
    kfree(arr2);
    kheap_print_stat();

    char *arr3 = (char *)kmalloc(5 * sizeof(char));
    s_printf("arr3 = %p", arr3);

    int *arr4 = (int *)kmalloc(8200 * sizeof(int));
    for (i = 0; i < 8200; i++)
        arr4[i] = i * 2;
    s_printf("\n");

    for (i = 0; i < 8200; i++)
        s_printf("%d ", arr4[i]);

    s_printf("(arr4) = %p\n", arr4);
    s_printf("(arr4-hdr) = 0x%x   heap_end = %p\n",
             (usz)arr4 - (usz)sizeof(kheap_item_t), kheap_end);

    kheap_print_stat();
    kfree(arr4);
    kheap_print_stat();
}