```c
/* Fixed‑buffer arena allocator – C11 */
#include <stddef.h>
#include <stdalign.h>
#include <stdint.h>
#include <string.h>

typedef struct block_header {
    size_t size;                 /* total block size (incl. header), LSB = alloc flag */
    struct block_header *prev;  /* valid only when block is free */
    struct block_header *next;  /* valid only when block is free */
} block_header;

/* arena state – lives in static storage, not in the buffer */
static unsigned char *arena_base = NULL;
static size_t arena_cap = 0;
static unsigned char *arena_end = NULL;
static block_header *free_head = NULL;

/* constants */
enum {
    ALIGN = alignof(max_align_t)
};

/* helpers */
static size_t align_up(size_t n) { return (n + ALIGN - 1) & ~(ALIGN - 1); }

static size_t hdr_size(void) {
    return align_up(sizeof(block_header));
}

static size_t blk_sz(const block_header *h) { return h->size & ~(size_t)1; }
static int    blk_alloc(const block_header *h) { return (int)(h->size & (size_t)1); }
static void   blk_set_alloc(block_header *h, int a) {
    h->size = (blk_sz(h) | (size_t)a);
}

/* free‑list manipulation */
static void free_insert(block_header *b) {
    b->prev = NULL;
    b->next = free_head;
    if (free_head) free_head->prev = b;
    free_head = b;
}
static void free_remove(block_header *b) {
    if (b->prev) b->prev->next = b->next;
    else        free_head = b->next;
    if (b->next) b->next->prev = b->prev;
    b->prev = b->next = NULL;
}

/* find previous block by linear scan (O(n), acceptable for simple arena) */
static block_header *find_prev(const block_header *cur) {
    block_header *it = (block_header *)arena_base;
    block_header *prev = NULL;
    while ((unsigned char *)it < (unsigned char *)cur) {
        size_t sz = blk_sz(it);
        block_header *next = (block_header *)((unsigned char *)it + sz);
        if (next == cur) { prev = it; break; }
        it = next;
    }
    return prev;
}

/* public API */
void arena_init(void *buf, size_t size) {
    /* align the start of the arena */
    uintptr_t start = (uintptr_t)buf;
    uintptr_t aligned = (start + ALIGN - 1) & ~(uintptr_t)(ALIGN - 1);
    if (aligned >= start + size) { /* not enough space after alignment */
        arena_base = NULL;
        arena_cap = 0;
        arena_end = NULL;
        free_head = NULL;
        return;
    }
    arena_base = (unsigned char *)aligned;
    arena_cap = size - (aligned - start);
    arena_end = arena_base + arena_cap;

    /* create a single free block covering the whole arena */
    if (arena_cap < hdr_size() + ALIGN) { /* too small to hold any block */
        free_head = NULL;
        return;
    }
    block_header *init = (block_header *)arena_base;
    init->size = arena_cap | (size_t)0;   /* not allocated */
    init->prev = init->next = NULL;
    free_head = init;
}

/* allocate n bytes, aligned for any type */
void *arena_alloc(size_t n) {
    if (n == 0 || arena_base == NULL) return NULL;
    n = align_up(n);
    size_t need = hdr_size() + n;

    for (block_header *b = free_head; b; b = b->next) {
        size_t bsz = blk_sz(b);
        if (bsz >= need) {
            /* possibly split */
            size_t remaining = bsz - need;
            if (remaining >= hdr_size() + ALIGN) {
                /* split: keep first part allocated, second part free */
                block_header *newfree = (block_header *)((unsigned char *)b + need);
                newfree->size = remaining | (size_t)0;
                free_insert(newfree);
                b->size = need | (size_t)1;
            } else {
                /* allocate whole block */
                free_remove(b);
                blk_set_alloc(b, 1);
            }
            return (void *)(b + 1);
        }
    }
    return NULL;    /* out of memory */
}

/* free a previously allocated block */
void arena_free(void *p) {
    if (p == NULL || arena_base == NULL) return;
    block_header *b = (block_header *)p - 1;
    blk_set_alloc(b, 0);
    free_insert(b);

    /* coalesce with next block */
    unsigned char *next_addr = (unsigned char *)b + blk_sz(b);
    if (next_addr < arena_end) {
        block_header *next = (block_header *)next_addr;
        if (!blk_alloc(next)) {
            free_remove(next);
            b->size = (blk_sz(b) + blk_sz(next)) | (size_t)0;
        }
    }

    /* coalesce with previous block */
    block_header *prev = find_prev(b);
    if (prev && !blk_alloc(prev)) {
        free_remove(b);          /* b will be merged into prev */
        prev->size = (blk_sz(prev) + blk_sz(b)) | (size_t)0;
    }
}

/* reallocate a block */
void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }

    n = align_up(n);
    block_header *b = (block_header *)p - 1;
    size_t old_payload = blk_sz(b) - hdr_size();

    /* shrink in place if possible */
    if (n <= old_payload) {
        size_t excess = old_payload - n;
        if (excess >= hdr_size() + ALIGN) {
            size_t new_sz = hdr_size() + n;
            size_t remain = blk