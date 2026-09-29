/* Fixed‑buffer arena allocator (C11) */
#include <stddef.h>
#include <stdalign.h>
#include <stdint.h>
#include <string.h>

#define ALIGN _Alignof(max_align_t)
#define ALIGN_UP(x)   (((x) + (ALIGN - 1)) & ~(ALIGN - 1))
#define MIN_BLOCK_SIZE (sizeof(block_header) + sizeof(size_t) + ALIGN)

/* block header – stored at the beginning of every block */
typedef struct block_header {
    size_t size;               /* total size (incl. header/footer), low bit = alloc flag */
} block_header;

/* arena state (kept in static variables) */
static unsigned char *arena_start = NULL;
static size_t arena_total = 0;
static block_header *free_list = NULL;

/* ----- helpers for free‑list manipulation (payload stores next pointer) ----- */
static block_header *free_next(const block_header *h) {
    return *(block_header * const *)((const void *)(h + 1));
}
static void free_set_next(block_header *h, block_header *n) {
    *(block_header **)((void *)(h + 1)) = n;
}
static void free_insert(block_header *h) {
    free_set_next(h, free_list);
    free_list = h;
}
static void free_remove(block_header *h, block_header *prev) {
    if (prev) free_set_next(prev, free_next(h));
    else      free_list = free_next(h);
}

/* ------------------------------------------------------------------------- */
void arena_init(void *buf, size_t size) {
    arena_start = (unsigned char *)buf;
    arena_total = size;
    free_list = NULL;

    /* align start address */
    uintptr_t start = (uintptr_t)arena_start;
    uintptr_t aligned = (start + (ALIGN - 1)) & ~(uintptr_t)(ALIGN - 1);
    size_t offset = aligned - start;
    if (offset >= arena_total) return;               /* not enough space */

    arena_start = (unsigned char *)aligned;
    arena_total -= offset;
    arena_total = ALIGN_UP(arena_total);
    if (arena_total < MIN_BLOCK_SIZE) return;        /* cannot hold a block */

    block_header *hdr = (block_header *)arena_start;
    hdr->size = arena_total;                         /* free */
    size_t *footer = (size_t *)(arena_start + arena_total - sizeof(size_t));
    *footer = hdr->size;
    free_insert(hdr);
}

/* ------------------------------------------------------------------------- */
void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t total = ALIGN_UP(n) + sizeof(block_header) + sizeof(size_t);
    if (total < MIN_BLOCK_SIZE) total = MIN_BLOCK_SIZE;

    for (block_header *curr = free_list, *prev = NULL; curr; prev = curr, curr = free_next(curr)) {
        size_t curr_sz = curr->size & ~(size_t)1;
        if (curr_sz >= total) {
            size_t remaining = curr_sz - total;
            if (remaining >= MIN_BLOCK_SIZE) {
                /* split */
                block_header *alloc = curr;
                alloc->size = total | (size_t)1;
                size_t *alloc_footer = (size_t *)((unsigned char *)alloc + total - sizeof(size_t));
                *alloc_footer = alloc->size;

                block_header *new_free = (block_header *)((unsigned char *)alloc + total);
                new_free->size = remaining;
                size_t *new_footer = (size_t *)((unsigned char *)new_free + remaining - sizeof(size_t));
                *new_footer = new_free->size;

                /* replace curr with new_free in free list */
                if (prev) free_set_next(prev, new_free);
                else      free_list = new_free;
                free_set_next(new_free, free_next(curr));
                return (void *)(alloc + 1);
            } else {
                /* use whole block */
                if (prev) free_remove(curr, prev);
                else      free_remove(curr, NULL);
                curr->size |= (size_t)1;
                size_t *footer = (size_t *)((unsigned char *)curr + curr_sz - sizeof(size_t));
                *footer = curr->size;
                return (void *)(curr + 1);
            }
        }
    }
    return NULL;    /* out of memory */
}

/* ------------------------------------------------------------------------- */
void arena_free(void *p) {
    if (!p) return;
    block_header *hdr = (block_header *)p - 1;
    size_t sz = hdr->size & ~(size_t)1;
    hdr->size = sz;                         /* mark free */
    size_t *footer = (size_t *)((unsigned char *)hdr + sz - sizeof(size_t));
    *footer = hdr->size;

    /* coalesce with previous block */
    if ((unsigned char *)hdr > arena_start) {
        size_t prev_sz = *((size_t *)((unsigned char *)hdr - sizeof(size_t)));
        block_header *prev_hdr = (block_header *)((unsigned char *)hdr - prev_sz);
        if ((prev_hdr->size & (size_t)1) == 0) {          /* previous free */
            /* remove previous from free list */
            for (block_header *c = free_list, *pr = NULL; c; pr = c, c = free_next(c))
                if (c == prev_hdr) { free_remove(c, pr); break; }
            /* merge */
            sz += prev_sz;
            prev_hdr->size = sz;
            footer = (size_t *)((unsigned char *)prev_hdr + sz - sizeof(size_t));
            *footer = prev_hdr->size;
            hdr = prev_hdr;
        }
    }

    /* coalesce with next block */
    unsigned char *next_ptr = (unsigned char *)hdr + sz;
    if (next_ptr < arena_start + arena_total) {
        block_header *next_hdr = (block_header *)next_ptr;
        size_t next_sz = next_hdr->size & ~(size_t)1;
        if ((next_hdr->size & (size_t)1) == 0) {          /* next free */
            /* remove next from free list */
            for (block_header *c = free_list, *pr = NULL; c; pr = c, c = free_next(c))
                if (c == next_hdr) { free_remove(c, pr); break; }
            /* merge */
            sz += next_sz;
            hdr->size = sz;
            footer = (size_t *)((unsigned char *)hdr + sz - sizeof(size_t));
            *footer = hdr->size;
        }
    }

    free_insert(hdr);
}

/* ------------------------------------------------------------------------- */
void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }

    block_header *hdr = (block_header *)p - 1;
    size_t old_sz = hdr->size & ~(size_t)1;
    size_t old_payload = old_sz - sizeof(block_header) - sizeof(size_t);
    size_t new_total = ALIGN_UP(n) + sizeof(block_header) + sizeof(size_t);
    if (new_total < MIN_BLOCK_SIZE) new_total = MIN_BLOCK_SIZE;

    if (new_total <= old_sz) {
        /* shrink, possibly split excess */
        size_t excess = old_sz - new_total;
        if (excess >= MIN_BLOCK_SIZE) {
            hdr->size = new_total | (size_t)1;
            size_t *new_footer = (size_t *)((unsigned char *)hdr + new_total - sizeof(size_t));
            *new_footer = hdr->size;

            block_header *new_free = (block_header *)((unsigned char *)hdr + new_total);
            new_free->size = excess;
            size_t *free_footer = (size_t *)((unsigned char *)new_free + excess - sizeof(size_t));
            *free_footer = new_free->size;
            arena_free((void *)(new_free + 1));
        }
        return p;
    }

    /* try to expand into next free block */
    unsigned char *next_ptr = (unsigned char *)hdr + old_sz;
    if (next_ptr < arena_start + arena_total) {
        block_header *next_hdr = (block_header *)next_ptr;
        size_t next_sz = next_hdr->size & ~(size_t)1;
        if ((next_hdr->size & (size_t)1) == 0 && (old_sz + next_sz) >= new_total) {
            /* remove next from free list */
            for (block_header *c = free_list, *pr = NULL; c; pr = c, c = free_next(c))
                if (c == next_hdr) { free_remove(c, pr); break; }

            size_t combined = old_sz + next_sz;
            size_t leftover = combined - new_total;

            if (leftover >= MIN_BLOCK_SIZE) {
                /* split leftover as a new free block */
                hdr->size = new_total | (size_t)1;
                size_t *new_footer = (size_t *)((unsigned char *)hdr + new_total - sizeof(size_t));
                *new_footer = hdr->size;

                block_header *rem = (block_header *)((unsigned char *)hdr + new_total);
                rem->size = leftover;
                size_t *rem_footer = (size_t *)((unsigned char *)rem + leftover - sizeof(size_t));
                *rem_footer = rem->size;
                arena_free((void *)(rem + 1));
            } else {
                /* consume whole next block */
                hdr->size = combined | (size_t)1;
                size_t *new_footer = (size_t *)((unsigned char *)hdr + combined - sizeof(size_t));
                *new_footer = hdr->size;
            }
            return p;
        }
    }

    /* fallback: allocate new block */
    void *newp = arena_alloc(n);
    if (!newp) return NULL;
    memcpy(newp, p, old_payload < n ? old_payload : n);
    arena_free(p);
    return newp;
}