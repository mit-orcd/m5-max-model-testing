/* Fixed‑buffer arena allocator (C11) */
#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>
#include <string.h>
#include <stdint.h>

#define ALIGN alignof(max_align_t)

static size_t align_up(size_t n, size_t a) {
    return (n + a - 1) & ~(a - 1);
}

/* Header stored at the beginning of every block */
typedef struct Header {
    size_t size;   /* total size of the block, including header */
    bool   free;   /* true if the block is free */
} Header;

/* Global arena state (all bookkeeping lives inside the arena) */
static void   *arena_base = NULL;
static size_t  arena_total = 0;
static const size_t HEADER_SIZE = align_up(sizeof(Header), ALIGN);

/* Initialise the arena with a buffer */
void arena_init(void *buf, size_t size) {
    uintptr_t addr = (uintptr_t)buf;
    uintptr_t aligned = (addr + ALIGN - 1) & ~(ALIGN - 1);
    size_t offset = aligned - addr;
    if (offset >= size) {                 /* buffer too small after alignment */
        arena_base = NULL;
        arena_total = 0;
        return;
    }
    arena_base = (void *)aligned;
    arena_total = size - offset;
    if (arena_total < HEADER_SIZE) {      /* not enough space for a block */
        arena_base = NULL;
        arena_total = 0;
        return;
    }
    Header *h = (Header *)arena_base;
    h->size = arena_total;
    h->free = true;
}

/* Allocate a block of at least n bytes, aligned for any type */
void *arena_alloc(size_t n) {
    if (n == 0 || arena_base == NULL) return NULL;
    size_t payload = align_up(n, ALIGN);
    size_t needed  = HEADER_SIZE + payload;

    Header *h = (Header *)arena_base;
    while ((char *)h < (char *)arena_base + arena_total) {
        if (h->free && h->size >= needed) {
            size_t excess = h->size - needed;
            if (excess >= HEADER_SIZE + ALIGN) {
                /* split the block */
                Header *next = (Header *)((char *)h + needed);
                next->size = excess;
                next->free = true;
                h->size = needed;
            }
            h->free = false;
            return (void *)(h + 1);
        }
        h = (Header *)((char *)h + h->size);
    }
    return NULL;   /* no suitable block */
}

/* Coalesce a block with its next neighbour if that neighbour is free */
static void coalesce_next(Header *h) {
    Header *next = (Header *)((char *)h + h->size);
    if ((char *)next < (char *)arena_base + arena_total && next->free) {
        h->size += next->size;
    }
}

/* Find the block that precedes h (or NULL if none) */
static Header *find_prev(Header *h) {
    Header *prev = NULL;
    Header *iter = (Header *)arena_base;
    while ((char *)iter < (char *)h) {
        Header *next = (Header *)((char *)iter + iter->size);
        if (next == h) {
            prev = iter;
            break;
        }
        iter = next;
    }
    return prev;
}

/* Free a previously allocated block */
void arena_free(void *p) {
    if (!p) return;
    Header *h = (Header *)p - 1;
    h->free = true;

    /* merge with next block */
    coalesce_next(h);

    /* merge with previous block */
    Header *prev = find_prev(h);
    if (prev && prev->free) {
        prev->size += h->size;
    }
}

/* Reallocate a block, preserving contents up to the smaller of old and new sizes */
void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    Header *h = (Header *)p - 1;
    size_t old_payload = h->size - HEADER_SIZE;
    size_t new_payload = align_up(n, ALIGN);
    size_t new_total   = HEADER_SIZE + new_payload;

    if (new_total <= h->size) {
        /* shrink in place, possibly split */
        size_t excess = h->size - new_total;
        if (excess >= HEADER_SIZE + ALIGN) {
            Header *free_block = (Header *)((char *)h + new_total);
            free_block->size = excess;
            free_block->free = true;
            h->size = new_total;
            coalesce_next(free_block);   /* merge with following free block if any */
        }
        return p;
    }

    /* try to grow into the next free block */
    Header *next = (Header *)((char *)h + h->size);
    size_t need_extra = new_total - h->size;
    if ((char *)next < (char *)arena_base + arena_total && next->free && next->size >= need_extra) {
        /* expand */
        if (next->size - need_extra >= HEADER_SIZE + ALIGN) {
            /* shrink the next block */
            Header *new_next = (Header *)((char *)next + need_extra);
            new_next->size = next->size - need_extra;
            new_next->free = true;
            h->size = new_total;
        } else {
            /* consume the whole next block */
            h->size += next->size;
        }
        return p;
    }

    /* allocate a new block, copy data, free old block */
    void *newp = arena_alloc(n);
    if (!newp) return NULL;
    size_t copy_sz = old_payload < n ? old_payload : n;
    memcpy(newp, p, copy_sz);
    arena_free(p);
    return newp;
}