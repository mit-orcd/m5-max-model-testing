#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

/* Internal header for each block */
typedef struct block_header {
    size_t size;          /* Size of the block including this header */
    int is_free;          /* 1 if free, 0 if allocated */
    struct block_header *next; /* Next block in the list (for traversal) */
} block_header_t;

/* Global state stored in the buffer */
static block_header_t *arena_head = NULL;
static size_t arena_total_size = 0;
static size_t arena_used_size = 0;

/* Helper: align up to max_align_t */
static size_t align_up(size_t n, size_t alignment) {
    return (n + alignment - 1) & ~(alignment - 1);
}

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(block_header_t)) {
        arena_head = NULL;
        arena_total_size = 0;
        arena_used_size = 0;
        return;
    }

    /* Align the start of the buffer to max_align_t */
    uintptr_t buf_addr = (uintptr_t)buf;
    size_t align = alignof(max_align_t);
    uintptr_t aligned_addr = (buf_addr + align - 1) & ~(align - 1);
    size_t offset = aligned_addr - buf_addr;

    if (offset + sizeof(block_header_t) > size) {
        arena_head = NULL;
        arena_total_size = 0;
        arena_used_size = 0;
        return;
    }

    block_header_t *first = (block_header_t *)aligned_addr;
    first->size = size - offset;
    first->is_free = 1;
    first->next = NULL;

    arena_head = first;
    arena_total_size = size - offset;
    arena_used_size = 0;
}

void *arena_alloc(size_t n) {
    if (n == 0 || !arena_head) {
        return NULL;
    }

    /* Align the requested size to max_align_t, plus header */
    size_t align = alignof(max_align_t);
    size_t header_size = align_up(sizeof(block_header_t), align);
    size_t total_needed = header_size + n;

    /* Find a free block that can fit */
    block_header_t *curr = arena_head;
    while (curr) {
        if (curr->is_free && curr->size >= total_needed) {
            /* Split the block if there's enough leftover */
            size_t leftover = curr->size - total_needed;
            if (leftover >= header_size + align) {
                /* Create a new free block after the allocated one */
                block_header_t *new_block = (block_header_t *)((char *)curr + total_needed);
                new_block->size = leftover;
                new_block->is_free = 1;
                new_block->next = curr->next;

                curr->size = total_needed;
                curr->is_free = 0;
                curr->next = new_block;
            } else {
                /* Use the entire block */
                curr->is_free = 0;
            }

            arena_used_size += curr->size;
            return (void *)((char *)curr + header_size);
        }
        curr = curr->next;
    }

    return NULL;
}

void arena_free(void *p) {
    if (!p || !arena_head) {
        return;
    }

    /* Find the block header for this pointer */
    size_t align = alignof(max_align_t);
    size_t header_size = align_up(sizeof(block_header_t), align);
    block_header_t *header = (block_header_t *)((char *)p - header_size);

    /* Verify it's a valid allocated block */
    if (!header->is_free) {
        header->is_free = 1;
        arena_used_size -= header->size;

        /* Merge with next block if free */
        if (header->next && header->next->is_free) {
            header->size += header->next->size;
            header->next = header->next->next;
        }

        /* Merge with previous block if free */
        block_header_t *prev = NULL;
        block_header_t *curr = arena_head;
        while (curr && curr != header) {
            prev = curr;
            curr = curr->next;
        }

        if (prev && prev->is_free) {
            prev->size += header->size;
            prev->next = header->next;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    size_t align = alignof(max_align_t);
    size_t header_size = align_up(sizeof(block_header_t), align);
    block_header_t *header = (block_header_t *)((char *)p - header_size);

    if (header->is_free) {
        /* Invalid: realloc on freed pointer */
        return NULL;
    }

    size_t old_data_size = header->size - header_size;
    size_t new_total_needed = header_size + n;

    /* Case 1: Try to grow in place if next block is free and large enough */
    if (header->next && header->next->is_free) {
        size_t combined_size = header->size + header->next->size;
        if (combined_size >= new_total_needed) {
            /* Check if we can split the combined block */
            size_t leftover = combined_size - new_total_needed;
            if (leftover >= header_size + align) {
                /* Split: keep new_total_needed for current, rest becomes free */
                block_header_t *new_free = (block_header_t *)((char *)header + new_total_needed);
                new_free->size = leftover;
                new_free->is_free = 1;
                new_free->next = header->next->next;

                header->size = new_total_needed;
                header->next = new_free;
            } else {
                /* Use all combined space */
                header->size = combined_size;
                header->next = header->next->next;
            }
            /* Data is already in place, no copy needed */
            return p;
        }
    }

    /* Case 2: Allocate new block and copy */
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) {
        return NULL;
    }

    size_t copy_size = old_data_size < n ? old_data_size : n;
    memcpy(new_ptr, p, copy_size);

    arena_free(p);

    return new_ptr;
}