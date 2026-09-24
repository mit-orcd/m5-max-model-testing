```c
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGN _Alignof(max_align_t)
#define ALIGN_UP(x)   (((x) + (ALIGN - 1)) & ~(ALIGN - 1))

static unsigned char *arena_base = NULL;
static size_t arena_size = 0;

#define FLAG_ALLOC  ((size_t)1)
#define SIZE_MASK   (~(size_t)0 - 1)

/*---------------------------------------------------------------*/
/* Initialise the arena with a buffer. All state lives inside it */
void arena_init(void *buf, size_t size)
{
    unsigned char *p = (unsigned char *)buf;

    /* align the start address up */
    uintptr_t up = ((uintptr_t)p + (ALIGN - 1)) & ~(uintptr_t)(ALIGN - 1);
    p = (unsigned char *)up;

    if (up - (uintptr_t)buf > size) {          /* no space after alignment */
        arena_base = NULL;
        arena_size = 0;
        return;
    }

    size = size - (up - (uintptr_t)buf);
    size &= ~(ALIGN - 1);                     /* round down to alignment */

    if (size < 2 * sizeof(size_t)) {          /* need at least header+footer */
        arena_base = NULL;
        arena_size = 0;
        return;
    }

    arena_base = p;
    arena_size = size;

    /* single free block covering the whole arena */
    *(size_t *)arena_base = size;                     /* header */
    *(size_t *)(arena_base + size - sizeof(size_t)) = size;   /* footer */
}

/*---------------------------------------------------------------*/
static size_t block_size(void *hdr) { return *(size_t *)hdr & SIZE_MASK; }
static int    block_alloc(void *hdr){ return (*(size_t *)hdr) & FLAG_ALLOC; }
static void   set_header(void *hdr, size_t sz, int alloc)
{
    *(size_t *)hdr = sz | (alloc ? FLAG_ALLOC : 0);
}
static void   set_footer(void *ftr, size_t sz, int alloc)
{
    *(size_t *)ftr = sz | (alloc ? FLAG_ALLOC : 0);
}

/*---------------------------------------------------------------*/
void *arena_alloc(size_t n)
{
    if (n == 0 || arena_base == NULL) return NULL;

    size_t payload = ALIGN_UP(n);
    size_t needed  = payload + 2 * sizeof(size_t);   /* header+footer */

    unsigned char *cur = arena_base;
    while (cur < arena_base + arena_size) {
        size_t hdr = *(size_t *)cur;
        size_t sz  = hdr & SIZE_MASK;
        int    alloc = hdr & FLAG_ALLOC;

        if (!alloc && sz >= needed) {
            size_t remaining = sz - needed;

            /* allocate front part */
            set_header(cur, needed, 1);
            set_footer(cur + needed - sizeof(size_t), needed, 1);

            if (remaining >= 2 * sizeof(size_t) + ALIGN) {
                unsigned char *next = cur + needed;
                set_header(next, remaining, 0);
                set_footer(next + remaining - sizeof(size_t), remaining, 0);
            }
            return (void *)(cur + sizeof(size_t));
        }
        cur += sz;
    }
    return NULL;    /* out of memory */
}

/*---------------------------------------------------------------*/
void arena_free(void *p)
{
    if (!p || arena_base == NULL) return;

    unsigned char *hdr = (unsigned char *)p - sizeof(size_t);
    size_t sz = block_size(hdr);

    /* mark this block free */
    set_header(hdr, sz, 0);
    set_footer(hdr + sz - sizeof(size_t), sz, 0);

    /* coalesce with previous block */
    if (hdr > arena_base) {
        size_t *prev_ftr = (size_t *)(hdr - sizeof(size_t));
        size_t prev_sz = *prev_ftr & SIZE_MASK;
        int    prev_alloc = *prev_ftr & FLAG_ALLOC;
        if (!prev_alloc) {
            unsigned char *prev_hdr = hdr - prev_sz;
            size_t new_sz = prev_sz + sz;
            set_header(prev_hdr, new_sz, 0);
            set_footer(prev_hdr + new_sz - sizeof(size_t), new_sz, 0);
            hdr = prev_hdr;
            sz = new_sz;
        }
    }

    /* coalesce with next block */
    unsigned char *next_hdr = hdr + sz;
    if (next_hdr < arena_base + arena_size) {
        size_t next_sz = block_size(next_hdr);
        int    next_alloc = block_alloc(next_hdr);
        if (!next_alloc) {
            size_t new_sz = sz + next_sz;
            set_header(hdr, new_sz, 0);
            set_footer(hdr + new_sz - sizeof(size_t), new_sz, 0);
        }
    }
}

/*---------------------------------------------------------------*/
void *arena_realloc(void *p, size_t n)
{
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    if (arena_base == NULL) return NULL;

    unsigned char *hdr = (unsigned char *)p - sizeof(size_t);
    size_t cur_sz = block_size(hdr);
    size_t cur_payload = cur_sz - 2 * sizeof(size_t);
    size_t new_payload = ALIGN_UP(n);
    size_t needed = new_payload + 2 * sizeof(size_t);

    /* shrink in place if possible */
    if (needed <= cur_sz) {
        size_t leftover = cur_sz - needed;
        set_header(hdr, needed, 1);
        set_footer(hdr + needed - sizeof(size_t), needed, 1);
        if (leftover >= 2 * sizeof(size_t) + ALIGN) {
            unsigned char *next = hdr + needed;
            set_header(next, leftover, 0);
            set_footer(next + leftover - sizeof(size_t), leftover, 0);
        }
        return p;
    }

    /* try to expand into next free block */
    unsigned char *next_hdr = hdr + cur_sz;
    if (next_hdr < arena_base + arena_size) {
        size_t next_sz = block_size(next_hdr);
        int    next_alloc = block_alloc(next_hdr);
        if (!next_alloc && cur_sz + next_sz >= needed) {
            size_t extra = needed - cur_sz;
            size_t remain = next_sz - extra;

            if (remain >= 2 * sizeof(size_t) + ALIGN) {
                unsigned char *new_next = next_hdr + extra;
                set_header(new_next, remain, 0);
                set_footer(new_next + remain - sizeof(size_t), remain, 0);
            }
            size_t new_sz = cur_sz + extra;
            set_header(hdr, new