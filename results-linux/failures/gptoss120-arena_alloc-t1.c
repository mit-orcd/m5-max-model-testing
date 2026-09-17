/* Fixed‑buffer arena allocator – C11 */
#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include <string.h>

#define ALIGN alignof(max_align_t)
#define FREE_FLAG ((size_t)1)

/* round up to multiple of ALIGN */
static inline size_t up_align(size_t n)
{
    return (n + ALIGN - 1) & ~(ALIGN - 1);
}

/* block header – lives inside the arena */
typedef struct block_header {
    size_t size;                     /* total block size (incl. header) |
                                        LSB = 1 → free, 0 → allocated */
    struct block_header *prev_free; /* only valid when block is free */
    struct block_header *next_free; /* only valid when block is free */
} block_header;

/* arena state (single global arena) */
static unsigned char *arena_base = NULL;
static size_t arena_sz = 0;
static block_header *free_head = NULL;

/* -------------------------------------------------------------------------- */
void arena_init(void *buf, size_t size)
{
    unsigned char *p = (unsigned char *)buf;

    /* align start of arena */
    size_t off = (uintptr_t)p % ALIGN;
    if (off) {
        size_t pad = ALIGN - off;
        p += pad;
        size -= pad;
    }
    /* make arena size a multiple of ALIGN */
    size = size & ~(ALIGN - 1);

    arena_base = p;
    arena_sz   = size;
    free_head  = NULL;

    if (size < sizeof(block_header))
        return;                     /* not enough space for even one block */

    block_header *first = (block_header *)arena_base;
    first->size = size | FREE_FLAG;
    first->prev_free = NULL;
    first->next_free = NULL;
    free_head = first;
}

/* -------------------------------------------------------------------------- */
static void insert_free(block_header *b)
{
    b->prev_free = NULL;
    b->next_free = free_head;
    if (free_head)
        free_head->prev_free = b;
    free_head = b;
}

static void remove_free(block_header *b)
{
    if (b->prev_free)
        b->prev_free->next_free = b->next_free;
    else
        free_head = b->next_free;
    if (b->next_free)
        b->next_free->prev_free = b->prev_free;
}

/* -------------------------------------------------------------------------- */
void *arena_alloc(size_t n)
{
    if (n == 0 || arena_base == NULL)
        return NULL;

    size_t need = up_align(n) + sizeof(block_header);
    block_header *b = free_head;

    while (b) {
        size_t bsize = b->size & ~FREE_FLAG;
        if (bsize >= need)
            break;
        b = b->next_free;
    }
    if (!b)
        return NULL;                /* no suitable block */

    size_t bsize = b->size & ~FREE_FLAG;
    remove_free(b);

    /* split if enough left for a new free block */
    if (bsize >= need + sizeof(block_header) + ALIGN) {
        block_header *rest = (block_header *)((unsigned char *)b + need);
        size_t rest_sz = bsize - need;
        rest->size = rest_sz | FREE_FLAG;
        insert_free(rest);
        bsize = need;
    }
    b->size = bsize;                /* clear FREE_FLAG → allocated */
    return (void *)((unsigned char *)b + sizeof(block_header));
}

/* -------------------------------------------------------------------------- */
static block_header *prev_block(block_header *b)
{
    if ((unsigned char *)b == arena_base)
        return NULL;
    block_header *cur = (block_header *)arena_base;
    block_header *prev = NULL;
    while ((unsigned char *)cur < (unsigned char *)b) {
        size_t sz = cur->size & ~FREE_FLAG;
        if ((unsigned char *)cur + sz == (unsigned char *)b) {
            prev = cur;
            break;
        }
        cur = (block_header *)((unsigned char *)cur + sz);
    }
    return prev;
}

static block_header *next_block(block_header *b)
{
    size_t sz = b->size & ~FREE_FLAG;
    unsigned char *next = (unsigned char *)b + sz;
    if (next >= arena_base + arena_sz)
        return NULL;
    return (block_header *)next;
}

/* -------------------------------------------------------------------------- */
void arena_free(void *p)
{
    if (!p || arena_base == NULL)
        return;

    block_header *b = (block_header *)((unsigned char *)p - sizeof(block_header));
    size_t bsize = b->size & ~FREE_FLAG;
    b->size = bsize | FREE_FLAG;   /* mark free */

    /* try to merge with next block */
    block_header *next = next_block(b);
    if (next && (next->size & FREE_FLAG)) {
        remove_free(next);
        bsize += next->size & ~FREE_FLAG;
        b->size = bsize | FREE_FLAG;
    }

    /* try to merge with previous block */
    block_header *prev = prev_block(b);
    if (prev && (prev->size & FREE_FLAG)) {
        remove_free(prev);
        bsize += prev->size & ~FREE_FLAG;
        prev->size = bsize | FREE_FLAG;
        b = prev;
    }

    insert_free(b);
}

/* -------------------------------------------------------------------------- */
void *arena_realloc(void *p, size_t n)
{
    if (p == NULL)
        return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (arena_base == NULL)
        return NULL;

    block_header *b = (block_header *)((unsigned char *)p - sizeof(block_header));
    size_t old_tot = b->size & ~FREE_FLAG;
    size_t old_user = old_tot - sizeof(block_header);
    size_t need_user = up_align(n);
    size_t need_tot = need_user + sizeof(block_header);

    if (need_tot <= old_tot) {                 /* shrink in place */
        size_t excess = old_tot - need_tot;
        if (excess >= sizeof(block_header) + ALIGN) {
            b->size = need_tot;                /* keep allocated flag cleared */
            block_header *newfree = (block_header *)((unsigned char *)b + need_tot);
            newfree->size = excess | FREE_FLAG;
            insert_free(newfree);
        } else {
            b->size = old_tot;                 /* keep whole block */
        }
        return p;
    }

    /* try to expand into next free block */
    block_header *next = next_block(b);
    if (next && (next->size & FREE_FLAG)) {
        size_t next_sz = next->size & ~FREE_FLAG;
        size_t combined = old_tot + next_sz;
        if (combined >= need_tot) {
            remove_free(next);
            size_t remaining = combined - need_tot;
            b->size = need_tot;                /* allocated */
            if (remaining >= sizeof(block_header) + ALIGN) {
                block_header *newfree = (block_header *)((unsigned char *)b + need_tot);
                newfree->size = remaining | FREE_FLAG;
                insert_free(newfree);
            }
            return p;
        }
    }

    /* allocate new block, copy, free old */
    void *newp = arena_alloc(n);
    if (!newp)
        return NULL;
    memcpy(newp, p, old_user < n ? old_user : n);
    arena_free(p);
    return newp;
}