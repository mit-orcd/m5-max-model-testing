#include <stddef.h>
#include <string.h>

#define ALIGN(size, align) (((size) + (align) - 1) & ~((align) - 1))
#define MAX_ALIGN _Alignof(max_align_t)

typedef struct block_hdr {
    size_t size;
    int free;
    struct block_hdr *next;
} block_hdr;

static void *g_buf = NULL;
static size_t g_size = 0;
static size_t g_total = 0;

void arena_init(void *buf, size_t size) {
    g_buf = buf;
    g_size = size;
    g_total = 0;
    if (size == 0) return;
    block_hdr *h = (block_hdr *)buf;
    h->size = size - sizeof(block_hdr);
    h->free = 1;
    h->next = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t aligned_size = ALIGN(n, MAX_ALIGN);
    block_hdr *curr = (block_hdr *)g_buf;
    block_hdr *best = NULL;

    while (curr) {
        if (curr->free && curr->size >= aligned_size) {
            if (!best || curr->size < best->size) {
                best = curr;
            }
            if (curr->size == aligned_size) break;
        }
        curr = curr->next;
    }

    if (!best) return NULL;

    size_t overhead = sizeof(block_hdr);
    size_t total_block = aligned_size + overhead;

    if (best->size >= aligned_size + sizeof(block_hdr) + MAX_ALIGN) {
        block_hdr *new_block = (block_hdr *)((char *)best + total_block);
        new_block->size = best->size - total_block;
        new_block->free = 1;
        new_block->next = best->next;
        best->next = new_block;
        best->size = aligned_size;
    }

    best->free = 0;
    g_total += best->size;
    return (void *)(best + 1);
}

void arena_free(void *p) {
    if (!p) return;
    block_hdr *h = (block_hdr *)p - 1;
    h->free = 1;
    g_total -= h->size;
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!p) return arena_alloc(n);

    block_hdr *h = (block_hdr *)p - 1;
    size_t old_size = h->size;
    size_t aligned_size = ALIGN(n, MAX_ALIGN);

    if (old_size >= aligned_size) {
        return p;
    }

    block_hdr *next_block = (block_hdr *)((char *)p + old_size);
    if (next_block->free) {
        size_t new_total = old_size + next_block->size;
        if (new_total >= aligned_size) {
            next_block->next = next_block->next ? next_block->next->next : NULL;
            h->size = aligned_size;
            if (new_total > aligned_size + sizeof(block_hdr)) {
                block_hdr *remaining = (block_hdr *)((char *)h + sizeof(block_hdr) + aligned_size);
                remaining->size = new_total - aligned_size - sizeof(block_hdr);
                remaining->free = 1;
                remaining->next = next_block->next;
                h->next = remaining;
            } else {
                h->next = next_block->next;
            }
            return p;
        }
    }

    void *new_p = arena_alloc(n);
    if (new_p) {
        size_t copy_size = (old_size < n) ? old_size : n;
        memcpy(new_p, p, copy_size);
        arena_free(p);
    }
    return new_p;
}