```c
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((a) - 1))

typedef struct block_t {
    size_t size;
    int is_free;
    struct block_t *next;
    struct block_t *prev;
} block_t;

static block_t *arena_header = NULL;
static size_t arena_capacity = 0;

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(block_t) * 2) {
        arena_header = NULL;
        arena_capacity = 0;
        return;
    }

    arena_capacity = size;
    size_t alignment = _Alignof(max_align_t);
    size_t block_size = ALIGN_UP(sizeof(block_t), alignment);

    arena_header = (block_t *)buf;
    arena_header->size = block_size;
    arena_header->is_free = 0; /* Header block is "allocated" as metadata */
    arena_header->next = NULL;
    arena_header->prev = NULL;

    /* Create the initial free block */
    block_t *first_free = (block_t *)((char *)buf + block_size);
    size_t remaining = size - block_size;
    size_t payload_size = remaining - block_size;
    if (payload_size < block_size) payload_size = 0;

    first_free->size = block_size;
    first_free->is_free = 1;
    first_free->prev = arena_header;
    first_free->next = NULL;

    arena_header->next = first_free;
}

static block_t *find_free_block(size_t req_size) {
    size_t alignment = _Alignof(max_align_t);
    size_t min_block_size = req_size + ALIGN_UP(sizeof(block_t), alignment);
    
    if (min_block_size > arena_capacity) return NULL;

    block_t *curr = arena_header->next;
    while (curr != NULL) {
        if (curr->is_free && curr->size >= min_block_size) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

static void split_block(block_t *block, size_t size_needed) {
    size_t alignment = _Alignof(max_align_t);
    size_t header_size = ALIGN_UP(sizeof(block_t), alignment);
    
    if (block->size > size_needed + header_size + header_size) {
        block_t *new_block = (block_t *)((char *)block + size_needed);
        new_block->size = block->size - size_needed;
        new_block->is_free = 1;
        new_block->prev = block;
        new_block->next = block->next;
        
        if (block->next) {
            block->next->prev = new_block;
        }
        block->next = new_block;
        block->size = size_needed;
    }
}

static void coalesce_neighbors(block_t *block) {
    /* Coalesce with next */
    if (block->next && block->next->is_free) {
        block_t *next = block->next;
        block->size += next->size;
        block->next = next->next;
        if (next->next) {
            next->next->prev = block;
        }
        /* We don't need to clear next as it's overwritten or ignored now */
    }

    /* Coalesce with prev */
    if (block->prev && block->prev->is_free) {
        block_t *prev = block->prev;
        prev->size += block->size;
        prev->next = block->next;
        if (block->next) {
            block->next->prev = prev;
        }
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    if (arena_header == NULL) return NULL;

    size_t alignment = _Alignof(max_align_t);
    size_t header_size = ALIGN_UP(sizeof(block_t), alignment);
    size_t total_needed = header_size + ALIGN_UP(n, alignment);

    block_t *block = find_free_block(total_needed);
    if (!block) return NULL;

    block->is_free = 0;
    
    /* Split if there is significant space left */
    if (block->size > total_needed + header_size) {
        split_block(block, total_needed);
    } else {
        block->size = total_needed;
    }

    return (void *)((char *)block + header_size);
}

void arena_free(void *p) {
    if (!p || arena_header == NULL) return;

    size_t alignment = _Alignof(max_align_t);
    size_t header_size = ALIGN_UP(sizeof(block_t), alignment);
    
    block_t *block = (block_t *)((char *)p - header_size);
    
    if (block < arena_header || (char *)block + block->size > (char *)arena_header + arena_capacity) {
        return; /* Sanity check */
    }

    block->is_free = 1;
    coalesce_neighbors(block);
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (p == NULL) {
        return arena_alloc(n);
    }
    if (arena_header == NULL) return NULL;

    size_t alignment = _Alignof(max_align_t);
    size_t header_size = ALIGN_UP(sizeof(block_t), alignment);
    block_t *block = (block_t *)((char *)p - header_size);
    size_t old_size = block->size - header_size; /* Payload size */
    size_t new_payload_size = ALIGN_UP(n, alignment);
    size_t total_needed = header_size + new_payload_size;

    /* Check if current block is large enough */
    if (block->size >= total_needed) {
        /* Check if we can shrink or just keep as is */
        if (block->size > total_needed + header_size) {
            /* Split off the extra space at the end */
            block_t *new_block = (block_t *)((char *)block + total_needed);
            new_block->size = block->size - total_needed;
            new_block->is_free = 1;
            new_block->prev = block;
            new_block->next = block->next;
            if (block->next) {
                block->next->prev = new_block;
            }
            block->next = new_block;
            block->size = total_needed;
        }
        return p;
    }

    /* Check if next block is free and large enough to grow into */
    if (block->next && block->next->is_free) {
        size_t combined_size = block->size + block->next->size;
        if (combined_size >= total_needed) {
            /* Grow into next */
            block_t *next = block->next;
            block->size += next->size;
            block->next = next->next;
            if (next->next) {
                next->next->prev = block;
            }
            
            /* If we didn't need the whole next block, split the remainder */
            if (block->size > total_needed + header_size) {
                split_block(block, total_needed);
            } else {
                block->size = total_needed;
            }
            
            /* Preserve data up to min(old, new) */
            size_t copy_len = (old_size < new_payload_size) ? old_size : new_payload_size;
            /* Data is already in place, just ensure we don't copy garbage if expanding */
            /* Actually, data is contiguous, no copy needed unless moving. */
            /* The requirement says "Contents up to the smaller... are preserved". 
               Since we grew in place, the old data is still there. 
               If