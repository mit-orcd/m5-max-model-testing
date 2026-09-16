#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

typedef struct {
    size_t size;
    int free;
} block_header;

static void *base = NULL;
static size_t total_size = 0;

static block_header *get_header(void *p) {
    return (block_header *)((uint8_t *)p - sizeof(block_header));
}

static void *get_data(block_header *h) {
    return (void *)((uint8_t *)h + sizeof(block_header));
}

static block_header *next_block(block_header *h) {
    return (block_header *)((uint8_t *)h + sizeof(block_header) + h->size);
}

static int is_last_block(block_header *h) {
    return (uint8_t *)next_block(h) >= (uint8_t *)base + total_size;
}

static void coalesce_with_next(block_header *h) {
    if (!is_last_block(h) && next_block(h)->free) {
        h->size += sizeof(block_header) + next_block(h)->size;
    }
}

static void coalesce_with_prev(block_header *h) {
    if (h != (block_header *)base) {
        block_header *prev = (block_header *)((uint8_t *)h - sizeof(block_header));
        while ((uint8_t *)prev >= (uint8_t *)base && prev->size == 0) {
            prev = (block_header *)((uint8_t *)prev - sizeof(block_header));
            if (prev < (block_header *)base) break;
        }
        if ((uint8_t *)prev >= (uint8_t *)base && prev->free && !is_last_block(prev) && next_block(prev) == h) {
            prev->size += sizeof(block_header) + h->size;
            h = prev;
        }
    }
}

void arena_init(void *buf, size_t size) {
    base = buf;
    total_size = size;
    if (size < sizeof(block_header)) return;
    block_header *h = (block_header *)base;
    h->size = size - sizeof(block_header);
    h->free = 1;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t align = _Alignof(max_align_t);
    size_t aligned_n = (n + align - 1) & ~(align - 1);
    block_header *h = (block_header *)base;
    while (!is_last_block(h)) {
        if (h->free && h->size >= aligned_n) {
            if (h->size - aligned_n >= sizeof(block_header) + 1) {
                block_header *next = next_block(h);
                next->size = h->size - aligned_n - sizeof(block_header);
                next->free = 1;
                h->size = aligned_n;
            }
            h->free = 0;
            return get_data(h);
        }
        h = next_block(h);
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    block_header *h = get_header(p);
    h->free = 1;
    coalesce_with_prev(h);
    coalesce_with_next(h);
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    size_t align = _Alignof(max_align_t);
    size_t aligned_n = (n + align - 1) & ~(align - 1);
    block_header *h = get_header(p);
    size_t old_size = h->size;
    if (!is_last_block(h)) {
        block_header *next = next_block(h);
        if (next->free) {
            size_t avail = old_size + sizeof(block_header) + next->size;
            if (avail >= aligned_n) {
                if (avail - aligned_n >= sizeof(block_header) + 1) {
                    block_header *new_next = next_block(h);
                    new_next->size = avail - aligned_n - sizeof(block_header);
                    new_next->free = 1;
                    h->size = aligned_n;
                } else {
                    h->size = avail;
                }
                return p;
            }
        }
    }
    void *newp = arena_alloc(n);
    if (newp == NULL) return NULL;
    size_t copy_size = old_size < aligned_n ? old_size : aligned_n;
    memcpy(newp, p, copy_size);
    arena_free(p);
    return newp;
}