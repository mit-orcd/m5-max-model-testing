#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdalign.h>

typedef struct block_header {
    size_t size;          // size of data region (excluding header), 0 if free
    struct block_header *next_free;  // next free block in free list
    struct block_header *prev_free;  // prev free block in free list
} block_header;

static void *arena_buffer = NULL;
static size_t arena_size = 0;
static block_header *free_list = NULL;

// Align up to _Alignof(max_align_t)
#define ALIGN_UP(size) (((size) + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1))

// Minimum block size (must hold at least the header and be non-zero)
#define MIN_BLOCK_SIZE ALIGN_UP(sizeof(block_header))

static void remove_from_free_list(block_header *blk) {
    if (!blk->next_free && !blk->prev_free) return;
    
    if (blk->prev_free) {
        blk->prev_free->next_free = blk->next_free;
    }
    if (blk->next_free) {
        blk->next_free->prev_free = blk->prev_free;
    }
    if (free_list == blk) {
        free_list = blk->next_free;
    }
    blk->next_free = NULL;
    blk->prev_free = NULL;
}

static void add_to_free_list(block_header *blk) {
    blk->next_free = free_list;
    blk->prev_free = NULL;
    if (free_list) {
        free_list->prev_free = blk;
    }
    free_list = blk;
}

void arena_init(void *buf, size_t size) {
    arena_buffer = buf;
    arena_size = size;
    free_list = NULL;
    
    if (size >= MIN_BLOCK_SIZE) {
        block_header *initial = (block_header *)buf;
        initial->size = 0; // mark as free
        initial->next_free = NULL;
        initial->prev_free = NULL;
        free_list = initial;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0 || !arena_buffer || arena_size < MIN_BLOCK_SIZE) {
        return NULL;
    }
    
    size_t aligned_n = ALIGN_UP(n);
    if (aligned_n == 0) {
        return NULL;
    }
    
    // First-fit search in free list
    for (block_header *blk = free_list; blk != NULL; blk = blk->next_free) {
        if (blk->size >= aligned_n) {
            // Found a suitable block
            size_t remaining = blk->size - aligned_n;
            remove_from_free_list(blk);
            
            if (remaining >= MIN_BLOCK_SIZE) {
                // Split the block
                block_header *next = (block_header *)((char *)blk + sizeof(block_header) + aligned_n);
                next->size = remaining - sizeof(block_header);
                next->next_free = NULL;
                next->prev_free = NULL;
                add_to_free_list(next);
                blk->size = aligned_n;
            } else {
                // Use entire block
                blk->size = blk->size; // keep full size (but now marked as allocated)
            }
            
            return (void *)((char *)blk + sizeof(block_header));
        }
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (!p || !arena_buffer) return;
    
    block_header *blk = (block_header *)((char *)p - sizeof(block_header));
    
    // Already free? (shouldn't happen in correct use, but be defensive)
    if (blk->size == 0) return;
    
    blk->size = 0; // mark as free
    add_to_free_list(blk);
    
    // Merge with next block if free
    block_header *next = (block_header *)((char *)blk + sizeof(block_header) + blk->size);
    if ((char *)next < (char *)arena_buffer + arena_size && next->size == 0) {
        remove_from_free_list(next);
        blk->size += sizeof(block_header) + next->size;
    }
    
    // Merge with previous block if free
    // To do this, we need to traverse from arena_buffer to find the previous block
    // (since we don't store backward pointer in allocation, but we have free list)
    // Alternative: check if any free block ends at our start
    for (block_header *prev = free_list; prev != NULL; prev = prev->next_free) {
        if (prev == blk) continue;
        // Check if prev's data region ends exactly where blk starts
        if ((char *)prev + sizeof(block_header) + prev->size == (char *)blk) {
            remove_from_free_list(prev);
            remove_from_free_list(blk);
            prev->size += sizeof(block_header) + blk->size;
            add_to_free_list(prev);
            break;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (!arena_buffer) return NULL;
    
    block_header *blk = (block_header *)((char *)p - sizeof(block_header));
    size_t old_size = blk->size;
    
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    size_t new_size = ALIGN_UP(n);
    if (new_size == 0) return NULL;
    
    // If same or smaller (and enough space remains to avoid fragmentation), just return
    if (new_size <= old_size && old_size - new_size < MIN_BLOCK_SIZE) {
        return p;
    }
    
    // Try to grow in-place if next block is free and large enough
    block_header *next = (block_header *)((char *)blk + sizeof(block_header) + blk->size);
    size_t total_available = blk->size;
    
    // Check if next block is free and adjacent
    if ((char *)next < (char *)arena_buffer + arena_size && next->size == 0) {
        total_available += sizeof(block_header) + next->size;
    }
    
    if (total_available >= new_size) {
        // Grow into next block
        if ((char *)next < (char *)arena_buffer + arena_size && next->size == 0) {
            remove_from_free_list(next);
            blk->size = total_available - sizeof(block_header);
        } else {
            // Already enough space, but need to adjust size
            if (old_size < new_size) {
                // No split needed if just expanding within same block
                blk->size = new_size;
            }
        }
        return p;
    }
    
    // Allocate new block and copy
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) return NULL;
    
    size_t copy_size = old_size < n ? old_size : n;
    memcpy(new_ptr, p, copy_size);
    arena_free(p);
    
    return new_ptr;
}