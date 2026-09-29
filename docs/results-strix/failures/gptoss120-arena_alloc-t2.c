```c
/* arena allocator – all bookkeeping lives inside the supplied buffer */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

#define ALIGN   (sizeof(max_align_t))
#define ALIGN_UP(x)   (((x) + (ALIGN-1)) & ~(ALIGN-1))

typedef struct {
    size_t size;                /* low bit = 1 if allocated, otherwise free */
} block_header;

static unsigned char *arena_start = NULL;
static size_t arena_total = 0;

/* header size rounded up to alignment so payload is always aligned */
enum { HEADER_SIZE = (sizeof(block_header) + ALIGN-1) & ~(ALIGN-1) };

#define BLOCK_SIZE(h)   ((h)->size & ~(size_t)1)
#define IS_ALLOCATED(h) ((h)->size & (size_t)1)
#define SET_ALLOCATED(h)   ((h)->size |= (size_t)1)
#define CLEAR_ALLOCATED(h) ((h)->size &= ~(size_t)1)

/* ---------- internal helpers ----------------------------------- */
static block_header *next_block(block_header *b)
{
    unsigned char *p = (unsigned char *)b + BLOCK_SIZE(b);
    if (p >= arena_start + arena_total) return NULL;
    return (block_header *)p;
}

static block_header *find_prev(block_header *target)
{
    block_header *cur = (block_header *)arena_start;
    block_header *prev = NULL;
    while (cur && cur != target) {
        prev = cur;
        cur = next_block(cur);
    }
    return prev;
}

/* --------------------------------------------------------------- */
void arena_init(void *buf, size_t size)
{
    if (!buf || size < HEADER_SIZE) {
        arena_start = NULL;
        arena_total = 0;
        return;
    }
    uintptr_t start = (uintptr_t)buf;
    uintptr_t aligned = ALIGN_UP(start);
    size_t offset = aligned - start;
    if (size <= offset + HEADER_SIZE) {
        arena_start = NULL;
        arena_total = 0;
        return;
    }
    arena_start = (unsigned char *)aligned;
    arena_total = ALIGN_UP(size - offset);
    if (arena_total < HEADER_SIZE) {
        arena_start = NULL;
        arena_total = 0;
        return;
    }
    block_header *first = (block_header *)arena_start;
    first->size = arena_total;                /* free block covering whole arena */
}

/* --------------------------------------------------------------- */
void *arena_alloc(size_t n)
{
    if (!arena_start || n == 0) return NULL;

    size_t needed = HEADER_SIZE + n;
    needed = ALIGN_UP(needed);

    block_header *cur = (block_header *)arena_start;
    while (cur) {
        if (!IS_ALLOCATED(cur) && BLOCK_SIZE(cur) >= needed) {
            size_t cur_sz = BLOCK_SIZE(cur);
            size_t excess = cur_sz - needed;

            if (excess >= HEADER_SIZE + ALIGN) {
                block_header *next = (block_header *)((unsigned char *)cur + needed);
                next->size = excess;                 /* free remainder */
                cur->size = needed | (size_t)1;       /* allocate needed part */
            } else {
                cur->size = cur_sz | (size_t)1;       /* allocate whole block */
            }
            return (void *)((unsigned char *)cur + HEADER_SIZE);
        }
        cur = next_block(cur);
    }
    return NULL;                                      /* out of memory */
}

/* --------------------------------------------------------------- */
void arena_free(void *p)
{
    if (!p) return;
    block_header *blk = (block_header *)((unsigned char *)p - HEADER_SIZE);
    CLEAR_ALLOCATED(blk);

    /* coalesce with next */
    block_header *next = next_block(blk);
    if (next && !IS_ALLOCATED(next)) {
        blk->size = (BLOCK_SIZE(blk) + BLOCK_SIZE(next));
    }

    /* coalesce with previous */
    block_header *prev = find_prev(blk);
    if (prev && !IS_ALLOCATED(prev)) {
        prev->size = (BLOCK_SIZE(prev) + BLOCK_SIZE(blk));
    }
}

/* --------------------------------------------------------------- */
void *arena_realloc(void *p, size_t n)
{
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }

    block_header *blk = (block_header *)((unsigned char *)p - HEADER_SIZE);
    size_t cur_payload = BLOCK_SIZE(blk) - HEADER_SIZE;

    if (n <= cur_payload) {                     /* shrink in place */
        size_t needed = HEADER_SIZE + n;
        needed = ALIGN_UP(needed);
        size_t cur_sz = BLOCK_SIZE(blk);
        size_t excess = cur_sz - needed;
        if (excess >= HEADER_SIZE + ALIGN) {
            block_header *newb = (block_header *)((unsigned char *)blk + needed);
            newb->size = excess;               /* free remainder */
            CLEAR_ALLOCATED(newb);
            blk->size = needed | (size_t)1;
        }
        return p;
    }

    /* try to grow into next free block */
    size_t needed = HEADER_SIZE + n;
    needed = ALIGN_UP(needed);
    block_header *next = next_block(blk);
    size_t cur_sz = BLOCK_SIZE(blk);

    if (next && !IS_ALLOCATED(next)) {
        size_t combined = cur_sz + BLOCK_SIZE(next);
        if (combined >= needed) {
            size_t excess = combined - needed;
            if (excess >= HEADER_SIZE + ALIGN) {
                block_header