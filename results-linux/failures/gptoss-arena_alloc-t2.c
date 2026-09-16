#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

/* Helper macro: round up to nearest multiple of ALIGN */
#define ALIGN_UP(x, ALIGN) (((x) + ((ALIGN)-1)) & ~((ALIGN)-1))

/* Block header structure */
typedef struct block_header {
    size_t size;          /* total block size including header & footer */
    unsigned char allocated; /* 0 = free, 1 = allocated */
    /* the rest of the header may contain padding to satisfy alignment */
} block_header;

/* Global arena state */
static void *arena_start = NULL;
static size_t arena_size  = 0;

static size_t ALIGN;          /* alignment for the arena */
static size_t HEADER_SIZE;    /* size of block_header, rounded to ALIGN */
static size_t FOOTER_SIZE;    /* size of footer (size_t) */
static size_t MIN_BLOCK_SIZE; /* minimal block size that can be split */

/* Initialise the arena with the supplied buffer */
void arena_init(void *buf, size_t size)
{
    /* Determine alignment */
    ALIGN = alignof(max_align_t);

    /* Align buffer start */
    uintptr_t addr = (uintptr_t)buf;
    addr = ALIGN_UP(addr, ALIGN);
    void *aligned_buf = (void *)addr;
    size_t aligned_size = size - (aligned_buf - buf);
    aligned_size = ALIGN_UP(aligned_size, ALIGN);

    /* Compute sizes */
    HEADER_SIZE = ALIGN_UP(sizeof(block_header), ALIGN);
    FOOTER_SIZE = sizeof(size_t);
    MIN_BLOCK_SIZE = ALIGN_UP(HEADER_SIZE + FOOTER_SIZE + 1, ALIGN);

    /* Set global state */
    arena_start = aligned_buf;
    arena_size  = aligned_size;

    /* Create a single free block covering the whole arena */
    block_header *hdr = (block_header *)arena_start;
    hdr->size = arena_size;
    hdr->allocated = 0;
    /* write footer */
    size_t *footer = (size_t *)((char *)arena_start + arena_size - FOOTER_SIZE);
    *footer = arena_size;
}

/* Allocate n bytes from the arena */
void *arena_alloc(size_t n)
{
    if (n == 0 || arena_start == NULL)
        return NULL;

    /* Align the request */
    size_t payload = ALIGN_UP(n, ALIGN);
    size_t needed = ALIGN_UP(HEADER_SIZE + payload + FOOTER_SIZE, ALIGN);

    /* Iterate over blocks */
    block_header *cur = (block_header *)arena_start;
    while ((char *)cur < (char *)arena_start + arena_size) {
        if (!cur->allocated && cur->size >= needed) {
            /* Found a suitable block */
            /* If it can be split */
            size_t leftover = cur->size - needed;
            if (leftover >= MIN_BLOCK_SIZE) {
                /* Split the block */
                block_header *next = (block_header *)((char *)cur + needed);
                next->size = leftover;
                next->allocated = 0;
                /* footer for the new free block */
                size_t *next_footer = (size_t *)((char *)next + leftover - FOOTER_SIZE);
                *next_footer = leftover;

                cur->size = needed;
                /* footer for the allocated block */
                size_t *footer = (size_t *)((char *)cur + needed - FOOTER_SIZE);
                *footer = needed;
            } else {
                /* Allocate the whole block */
                cur->allocated = 1;
                /* footer already exists, just update size if needed */
                size_t *footer = (size_t *)((char *)cur + cur->size - FOOTER_SIZE);
                *footer = cur->size;
            }
            return (void *)((char *)cur + HEADER_SIZE);
        }
        /* move to next block */
        cur = (block_header *)((char *)cur + cur->size);
    }
    return NULL; /* no suitable block */
}

/* Free a previously allocated block */
void arena_free(void *p)
{
    if (!p || arena_start == NULL)
        return;

    block_header *hdr = (block_header *)((char *)p - HEADER_SIZE);
    hdr->allocated = 0;

    /* Merge with previous block if free */
    if ((char *)hdr > (char *)arena_start + HEADER_SIZE) {
        size_t *prev_footer = (size_t *)((char *)hdr - FOOTER_SIZE);
        size_t prev_size = *prev_footer;
        block_header *prev_hdr = (block_header *)((char *)hdr - prev_size);
        if (!prev_hdr->allocated) {
            /* Merge prev_hdr and hdr */
            prev_hdr->size += hdr->size;
            /* update footer */
            size_t *footer = (size_t *)((char *)prev_hdr + prev_hdr->size - FOOTER_SIZE);
            *footer = prev_hdr->size;
            hdr = prev_hdr;
        }
    }

    /* Merge with next block if free */
    block_header *next_hdr = (block_header *)((char *)hdr + hdr->size);
    if ((char *)next_hdr < (char *)arena_start + arena_size && !next_hdr->allocated) {
        hdr->size += next_hdr->size;
        /* update footer */
        size_t *footer = (size_t *)((char *)hdr + hdr->size - FOOTER_SIZE);
        *footer = hdr->size;
    } else {
        /* Update footer for the (possibly merged) block */
        size_t *footer = (size_t *)((char *)hdr + hdr->size - FOOTER_SIZE);
        *footer = hdr->size;
    }
}

/* Reallocate a block to a new size */
void *arena_realloc(void *p, size_t n)
{
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (arena_start == NULL)
        return NULL;

    block_header *hdr = (block_header *)((char *)p - HEADER_SIZE);
    size_t old_payload = hdr->size - HEADER_SIZE - FOOTER_SIZE;

    /* Align the new request */
    size_t payload = ALIGN_UP(n, ALIGN);
    size_t needed = ALIGN_UP(HEADER_SIZE + payload + FOOTER_SIZE, ALIGN);

    /* Shrink in place if possible */
    if (hdr->size >= needed) {
        size_t leftover = hdr->size - needed;
        if (leftover >= MIN_BLOCK_SIZE) {
            /* Split the block */
            block_header *free_hdr = (block_header *)((char *)hdr + needed);
            free_hdr->size = leftover;
            free_hdr->allocated = 0;
            /* footer for the new free block */
            size_t *free_footer = (size_t *)((char *)free_hdr + leftover - FOOTER_SIZE);
            *free_footer = leftover;

            hdr->size = needed;
            /* update footer */
            size_t *footer = (size_t *)((char *)hdr + needed - FOOTER_SIZE);
            *footer = needed;
        }
        return p;
    }

    /* Try to grow into next free block */
    block_header *next_hdr = (block_header *)((char *)hdr + hdr->size);
    if ((char *)next_hdr < (char *)arena_start + arena_size &&
        !next_hdr->allocated &&
        hdr->size + next_hdr->size >= needed) {
        /* Merge */
        hdr->size += next_hdr->size;
        /* update footer */
        size_t *footer = (size_t *)((char *)hdr + hdr->size - FOOTER_SIZE);
        *footer = hdr->size;

        /* Now shrink if needed */
        size_t leftover = hdr->size - needed;
        if (leftover >= MIN_BLOCK_SIZE) {
            block_header *free_hdr = (block_header *)((char *)hdr + needed);
            free_hdr->size = leftover;
            free_hdr->allocated = 0;
            size_t *free_footer = (size_t *)((char *)free_hdr + leftover - FOOTER_SIZE);
            *free_footer = leftover;

            hdr->size = needed;
            footer = (size_t *)((char *)hdr + needed - FOOTER_SIZE);
            *footer = needed;
        }
        return p;
    }

    /* Allocate a new block */
    void *newp = arena_alloc(n);
    if (!newp)
        return NULL;
    size_t copy_size = old_payload < n ? old_payload : n;
    memcpy(newp, p, copy_size);
    arena_free(p);
    return newp;
}