#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*
 * Arena Allocator Implementation
 *
 * Uses an implicit free list with boundary tags stored inside the buffer.
 * Each block consists of a header and an optional footer (for the previous block).
 * The header contains:
 *   - size: total size of the block (header + payload + footer if present)
 *   - is_free: flag indicating if the block is free
 *
 * Alignment is handled by padding the payload size to ensure the next block's header
 * is aligned to _Alignof(max_align_t).
 */

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

/* Minimum block size to hold header and footer */
#define MIN_BLOCK_SIZE (ALIGN_UP(sizeof(size_t) * 2))

typedef struct {
    size_t size;      /* Total size of the block including headers */
    int is_free;      /* 1 if free, 0 if allocated */
} BlockHeader;

/* Get the header for a block given its payload pointer */
static inline BlockHeader *get_header(void *ptr) {
    return (BlockHeader *)((char *)ptr - sizeof(BlockHeader));
}

/* Get the payload pointer for a block given its header */
static inline void *get_payload(BlockHeader *header) {
    return (void *)((char *)header + sizeof(BlockHeader));
}

/* Get the next block's header */
static inline BlockHeader *get_next_block(BlockHeader *header, size_t arena_size) {
    BlockHeader *next = (BlockHeader *)((char *)header + header->size);
    if ((char *)next >= (char *)arena_base + arena_size - sizeof(BlockHeader)) {
        return NULL;
    }
    return next;
}

/* Get the previous block's header by looking at the footer of the current block */
static inline BlockHeader *get_prev_block(BlockHeader *header) {
    if (header == (BlockHeader *)arena_base) {
        return NULL;
    }
    /* The footer of the previous block is just before the current header */
    size_t *footer = (size_t *)((char *)header - sizeof(size_t));
    return (BlockHeader *)((char *)header - *footer);
}

static void *arena_base;
static size_t arena_size;

void arena_init(void *buf, size_t size) {
    if (!buf || size < MIN_BLOCK_SIZE + sizeof(BlockHeader)) {
        arena_base = NULL;
        arena_size = 0;
        return;
    }

    arena_base = buf;
    arena_size = size;

    /* Initialize the first block as free */
    BlockHeader *header = (BlockHeader *)arena_base;
    header->size = size - sizeof(BlockHeader);
    header->is_free = 1;

    /* Add a footer to the first block so we can find the previous block */
    size_t *footer = (size_t *)((char *)header + header->size);
    *footer = header->size;
}

void *arena_alloc(size_t n) {
    if (n == 0 || !arena_base) {
        return NULL;
    }

    /* Align the requested size */
    size_t aligned_n = ALIGN_UP(n);
    if (aligned_n < MIN_BLOCK_SIZE) {
        aligned_n = MIN_BLOCK_SIZE;
    }

    /* Search for a free block using first-fit */
    BlockHeader *current = (BlockHeader *)arena_base;
    while (current) {
        if (current->is_free && current->size >= aligned_n) {
            /* Split the block if there's enough space for another block */
            size_t remaining = current->size - aligned_n;
            if (remaining >= MIN_BLOCK_SIZE + sizeof(BlockHeader)) {
                /* Create a new free block after the allocated portion */
                BlockHeader *new_block = (BlockHeader *)((char *)current + aligned_n);
                new_block->size = remaining - sizeof(BlockHeader);
                new_block->is_free = 1;

                /* Update the current block's size */
                current->size = aligned_n;

                /* Add footer to the new block */
                size_t *footer = (size_t *)((char *)new_block + new_block->size);
                *footer = new_block->size;
            }

            current->is_free = 0;
            return get_payload(current);
        }
        current = get_next_block(current, arena_size);
    }

    return NULL;
}

void arena_free(void *p) {
    if (!p || !arena_base) {
        return;
    }

    BlockHeader *header = get_header(p);
    header->is_free = 1;

    /* Merge with the next block if it's free */
    BlockHeader *next = get_next_block(header, arena_size);
    if (next && next->is_free) {
        header->size += next->size + sizeof(BlockHeader);
        /* Remove the footer of the merged block */
        size_t *footer = (size_t *)((char *)header + header->size);
        *footer = header->size;
    }

    /* Merge with the previous block if it's free */
    BlockHeader *prev = get_prev_block(header);
    if (prev && prev->is_free) {
        prev->size += header->size + sizeof(BlockHeader);
        /* Remove the footer of the merged block */
        size_t *footer = (size_t *)((char *)prev + prev->size);
        *footer = prev->size;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    if (!p) {
        return arena_alloc(n);
    }

    if (!arena_base) {
        return NULL;
    }

    BlockHeader *header = get_header(p);
    size_t old_size = header->size;
    size_t aligned_n = ALIGN_UP(n);
    if (aligned_n < MIN_BLOCK_SIZE) {
        aligned_n = MIN_BLOCK_SIZE;
    }

    /* Check if we can expand into the next free block */
    BlockHeader *next = get_next_block(header, arena_size);
    if (next && next->is_free && (old_size + next->size + sizeof(BlockHeader)) >= aligned_n) {
        /* Merge with the next block */
        header->size += next->size + sizeof(BlockHeader);
        header->is_free = 0;

        /* Remove the footer of the merged block */
        size_t *footer = (size_t *)((char *)header + header->size);
        *footer = header->size;

        return p;
    }

    /* Allocate a new block and copy data */
    void *new_p = arena_alloc(n);
    if (!new_p) {
        return NULL;
    }

    /* Copy the minimum of old and new sizes */
    size_t copy_size = old_size < n ? old_size : n;
    memcpy(new_p, p, copy_size);

    arena_free(p);
    return new_p;
}