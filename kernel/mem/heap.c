#include "heap.h"
#include "vmm.h"
#include "pmm.h"
#include "mem.h"

struct HeapBlock {
    usize size;
    bool free;
    struct HeapBlock* next;
};

static struct HeapBlock* g_head;
static u32 g_heap_end;

static usize align_up(usize value, usize align) {
    if (align == 0U) {
        return value;
    }
    return (value + align - 1U) & ~(align - 1U);
}

static bool heap_expand(usize min_bytes) {
    usize alloc = align_up(min_bytes, PAGE_SIZE);
    for (usize off = 0; off < alloc; off += PAGE_SIZE) {
        u32 frame = pmm_alloc_frame();
        if (frame == 0U) {
            return false;
        }
        if (!vmm_map_page(g_heap_end + (u32)off, frame, PAGE_RW)) {
            return false;
        }
    }
    struct HeapBlock* block = (struct HeapBlock*)(u32)g_heap_end;
    block->size = alloc - sizeof(struct HeapBlock);
    block->free = true;
    block->next = 0;

    struct HeapBlock* tail = g_head;
    while (tail->next != 0) {
        tail = tail->next;
    }
    tail->next = block;
    g_heap_end += (u32)alloc;
    return true;
}

void heap_init(void) {
    g_heap_end = KHEAP_BASE;
    for (u32 off = 0; off < KHEAP_INITIAL_SIZE; off += PAGE_SIZE) {
        u32 frame = pmm_alloc_frame();
        POOS_ASSERT(frame != 0U);
        POOS_ASSERT(vmm_map_page(KHEAP_BASE + off, frame, PAGE_RW));
    }

    g_head = (struct HeapBlock*)(u32)KHEAP_BASE;
    g_head->size = KHEAP_INITIAL_SIZE - sizeof(struct HeapBlock);
    g_head->free = true;
    g_head->next = 0;
    g_heap_end = KHEAP_BASE + KHEAP_INITIAL_SIZE;
}

void* kmalloc(usize size, usize align) {
    if (size == 0U) {
        return 0;
    }

    usize needed = size;
    struct HeapBlock* cur = g_head;
    while (cur != 0) {
        if (cur->free && cur->size >= needed) {
            usize header_end = (usize)cur + sizeof(struct HeapBlock);
            usize aligned = align_up(header_end, align == 0U ? 8U : align);
            usize pad = aligned - header_end;
            if (cur->size < needed + pad) {
                cur = cur->next;
                continue;
            }
            cur->free = false;
            return (void*)aligned;
        }
        cur = cur->next;
    }

    if (!heap_expand(size + sizeof(struct HeapBlock))) {
        return 0;
    }
    return kmalloc(size, align);
}

void kfree(void* ptr) {
    if (ptr == 0) {
        return;
    }
    struct HeapBlock* cur = g_head;
    while (cur != 0) {
        usize start = (usize)cur + sizeof(struct HeapBlock);
        usize end = start + cur->size;
        if ((usize)ptr >= start && (usize)ptr < end) {
            POOS_ASSERT(!cur->free);
            cur->free = true;
            return;
        }
        cur = cur->next;
    }
    POOS_ASSERT(false);
}

void vga_write(const char* s);
void vga_write_u32(u32 value);

void heap_smoke_test(void) {
    u8* a = (u8*)kmalloc(16U, 8U);
    u8* b = (u8*)kmalloc(4096U, 4096U);
    u8* c = (u8*)kmalloc(65536U, 16U);
    POOS_ASSERT(a != 0 && b != 0 && c != 0);
    POOS_ASSERT(((u32)b & 0xFFFU) == 0U);

    for (u32 i = 0; i < 16U; ++i) { a[i] = (u8)i; }
    for (u32 i = 0; i < 16U; ++i) { POOS_ASSERT(a[i] == (u8)i); }

    kfree(a);
    u8* d = (u8*)kmalloc(16U, 8U);
    POOS_ASSERT(d == a);

    vga_write("Heap smoke tests passed: ptr16=");
    vga_write_u32((u32)d);
    vga_write(" ptr4k=");
    vga_write_u32((u32)b);
    vga_write(" ptr64k=");
    vga_write_u32((u32)c);
    vga_write("\n");
}
