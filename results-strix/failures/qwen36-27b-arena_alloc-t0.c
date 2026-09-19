```c
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

#define ARENA_ALIGNMENT _Alignof(max_align_t)

/* Round up size to be a multiple of alignment */
static size_t align_up(size_t size) {
    return (size + ARENA_ALIGNMENT - 1) & ~(ARENA_ALIGNMENT - 1);
}

/* Header structure for each block */
typedef struct {
    size_t size;      /* Total size of the block including headers */
    int is_free;      /* 1 if free, 0 if allocated */
} BlockHeader;

/* Minimum block size: must fit at least the header and be aligned */
#define MIN_BLOCK_SIZE align_up(sizeof(BlockHeader))

/* Pointer to the start of the arena buffer */
static void *arena_buf = NULL;
/* Size of the arena buffer */
static size_t arena_size = 0;

/* Get pointer to the header given a payload pointer */
static BlockHeader *get_header(void *ptr) {
    return (BlockHeader *)((char *)ptr - sizeof(BlockHeader));
}

/* Get pointer to the payload given a header pointer */
static void *get_payload(BlockHeader *header) {
    return (void *)((char *)header + sizeof(BlockHeader));
}

/* Get pointer to the next block's header */
static BlockHeader *get_next_block(BlockHeader *header) {
    return (BlockHeader *)((char *)header + header->size);
}

/* Get pointer to the previous block's header (using footer) */
static BlockHeader *get_prev_block(BlockHeader *header) {
    if ((char *)header == (char *)arena_buf) {
        return NULL;
    }
    /* The footer of the previous block is a copy of its header */
    BlockHeader *footer = (BlockHeader *)((char *)header - sizeof(BlockHeader));
    return footer;
}

/* Initialize the footer of a block (copy of the header) */
static void set_footer(BlockHeader *header) {
    BlockHeader *footer = (BlockHeader *)((char *)header + header->size - sizeof(BlockHeader));
    footer->size = header->size;
    footer->is_free = header->is_free;
}

/* Initialize the arena */
void arena_init(void *buf, size_t size) {
    arena_buf = buf;
    arena_size = size;

    if (buf == NULL || size < MIN_BLOCK_SIZE) {
        return;
    }

    /* Create a single free block spanning the entire buffer */
    BlockHeader *header = (BlockHeader *)buf;
    header->size = size;
    header->is_free = 1;
    set_footer(header);
}

/* Allocate memory from the arena */
void *arena_alloc(size_t n) {
    if (n == 0) {
        return NULL;
    }

    if (arena_buf == NULL) {
        return NULL;
    }

    /* Calculate the required block size: header + aligned payload + footer */
    size_t payload_size = align_up(n);
    size_t block_size = payload_size + 2 * sizeof(BlockHeader);

    /* Ensure minimum block size */
    if (block_size < MIN_BLOCK_SIZE + 2 * sizeof(BlockHeader)) {
        block_size = MIN_BLOCK_SIZE + 2 * sizeof(BlockHeader);
    }

    /* First-fit search through the free list */
    BlockHeader *current = (BlockHeader *)arena_buf;
    while (current != NULL && (char *)current < (char *)arena_buf + arena_size) {
        if (current->is_free && current->size >= block_size) {
            /* Split the block if there's enough space for another minimum block */
            size_t remaining = current->size - block_size;
            if (remaining >= MIN_BLOCK_SIZE + 2 * sizeof(BlockHeader)) {
                /* Create a new free block after the allocated one */
                BlockHeader *new_block = get_next_block(current);
                new_block->size = remaining;
                new_block->is_free = 1;
                set_footer(new_block);

                /* Update current block size */
                current->size = block_size;
                set_footer(current);
            }

            /* Mark the block as allocated */
            current->is_free = 0;
            set_footer(current);

            return get_payload(current);
        }
        current = get_next_block(current);
    }

    return NULL;
}

/* Free a block allocated by arena_alloc */
void arena_free(void *p) {
    if (p == NULL) {
        return;
    }

    if (arena_buf == NULL) {
        return;
    }

    BlockHeader *header = get_header(p);

    /* Mark the block as free */
    header->is_free = 1;
    set_footer(header);

    /* Coalesce with the next block if it's free */
    BlockHeader *next = get_next_block(header);
    if (next != NULL && (char *)next < (char *)arena_buf + arena_size && next->is_free) {
        header->size += next->size;
        set_footer(header);
    }

    /* Coalesce with the previous block if it's free */
    BlockHeader *prev = get_prev_block(header);
    if (prev != NULL && prev->is_free) {
        prev->size += header->size;
        set_footer(prev);
    }
}

/* Reallocate memory */
void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    if (p == NULL) {
        return arena_alloc(n);
    }

    if (arena_buf == NULL) {
        return NULL;
    }

    BlockHeader *header = get_header(p);
    size_t old_size = header->size;
    size_t old_payload_size = old_size - 2 * sizeof(BlockHeader);

    /* Calculate the required block size */
    size_t payload_size = align_up(n);
    size_t block_size = payload_size + 2 * sizeof(BlockHeader);

    /* Ensure minimum block size */
    if (block_size < MIN_BLOCK_SIZE + 2 * sizeof(BlockHeader)) {
        block_size = MIN_BLOCK_SIZE + 2 * sizeof(BlockHeader);
    }

    /* Check if we can expand into the next free block */
    BlockHeader *next = get_next_block(header);
    if (next != NULL && (char *)next < (char *)arena_buf + arena_size && next->is_free) {
        size_t combined_size = header->size + next->size;
        if (combined_size >= block_size) {
            /* Merge with the next block */
            header->size = combined_size;
            set_footer(header);

            /* If there's still enough space, split off the remainder */
            size_t remaining = combined_size - block_size;
            if (remaining >= MIN_BLOCK_SIZE + 2 * sizeof(BlockHeader)) {
                BlockHeader *new_block = get_next_block(header);
                new_block->size = remaining;
                new_block->is_free = 1;
                set_footer(new_block);

                header->size = block_size;
                set_footer(header);
            }

            return p;
        }
    }

    /* Otherwise, allocate a new block and copy data */
    void *new_p = arena_alloc(n);
    if (new_p == NULL) {
        return NULL;
    }

    /* Copy the minimum of old and new payload sizes */