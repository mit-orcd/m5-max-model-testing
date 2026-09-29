```c
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>

typedef struct block_header {
    size_t size;              // size of the block including header (0 if free)
    struct block_header *next_free;  // next free block in free list (NULL if last)
} block_header_t;

static void *arena_buffer = NULL;
static size_t arena_size = 0;
static block_header_t *free_list = NULL;

#define ALIGN_UP(p, a) ((void *)(((uintptr_t)(p) + (a) - 1) & ~((a) - 1)))
#define MIN_BLOCK_SIZE (sizeof(block_header_t) + _Alignof(max_align_t))
#define HEADER_SIZE (sizeof(block_header_t))

static void *find_free_block(size_t size) {
    block_header_t *prev = NULL;
    block_header_t *curr = free_list;
    while (curr != NULL) {
        if (curr->size >= size) {
            // Found a suitable block
            if (prev == NULL) {
                free_list = curr->next_free;
            } else {
                prev->next_free = curr->next_free;
            }
            return curr;
        }
        prev = curr;
        curr = curr->next_free;
    }
    return NULL;
}

static void coalesce(block_header_t *block) {
    // Coalesce with next block if it's free
    char *end = (char *)block + block->size;
    block_header_t *next = (block_header_t *)end;
    
    if (next != (block_header_t *)((char *)arena_buffer + arena_size) && next->size == 0) {
        // Merge with next block
        block->size += HEADER_SIZE + next->size;
        block->next_free = next->next_free;
    }
    
    // Coalesce with previous free block if adjacent
    // Since we don't have prev pointer, search from start of free list
    block_header_t *prev_free = NULL;
    block_header_t *curr = free_list;
    while (curr != NULL) {
        char *curr_end = (char *)curr + curr->size;
        if (curr_end == (char *)block) {
            // Merge with current free block
            curr->size += HEADER_SIZE + block->size;
            curr->next_free = block->next_free;
            if (prev_free == NULL) {
                free_list = curr;
            } else {
                prev_free->next_free = curr->next_free;
            }
            block = curr;
            break;
        }
        prev_free = curr;
        curr = curr->next_free;
    }
}

void arena_init(void *buf, size_t size) {
    if (buf == NULL || size < MIN_BLOCK_SIZE) {
        arena_buffer = NULL;
        arena_size = 0;
        free_list = NULL;
        return;
    }
    
    arena_buffer = buf;
    arena_size = size;
    
    // Create initial free block spanning the whole buffer
    free_list = (block_header_t *)buf;
    free_list->size = 0; // Mark as free
    free_list->next_free = NULL;
    
    // Ensure proper alignment of the buffer start
    size_t aligned_offset = (size_t)((char *)ALIGN_UP(buf, _Alignof(max_align_t)) - (char *)buf);
    if (aligned_offset > 0) {
        // Move the free block to be properly aligned
        free_list = (block_header_t *)((char *)buf + aligned_offset);
        size_t adjusted_size = size - aligned_offset;
        if (adjusted_size >= MIN_BLOCK_SIZE) {
            free_list->size = adjusted_size - HEADER_SIZE;
            free_list->next_free = NULL;
        } else {
            // Buffer too small even after alignment
            arena_buffer = NULL;
            arena_size = 0;
            free_list = NULL;
        }
    } else {
        // Buffer is already aligned
        if (size >= MIN_BLOCK_SIZE) {
            free_list->size = size - HEADER_SIZE;
        } else {
            arena_buffer = NULL;
            arena_size = 0;
            free_list = NULL;
        }
    }
}

void *arena_alloc(size_t n) {
    if (n == 0 || arena_buffer == NULL) {
        return NULL;
    }
    
    // Calculate required size including header and alignment
    size_t total_size = HEADER_SIZE + n;
    size_t aligned_header = (size_t)((char *)ALIGN_UP((void *)free_list, _Alignof(max_align_t)) - (char *)free_list);
    total_size += aligned_header - HEADER_SIZE; // adjust for header alignment
    
    // Ensure alignment for data region
    total_size = (size_t)ALIGN_UP((char *)free_list + HEADER_SIZE, _Alignof(max_align_t)) - (char *)free_list + n;
    
    block_header_t *block = find_free_block(total_size);
    if (block == NULL) {
        return NULL;
    }
    
    // Split block if there's enough space left
    size_t remaining = block->size - total_size;
    if (remaining >= MIN_BLOCK_SIZE) {
        // Split the block
        block_header_t *new_block = (block_header_t *)((char *)block + total_size);
        new_block->size = remaining - HEADER_SIZE;
        new_block->next_free = free_list;
        free_list = new_block;
    }
    
    block->size = total_size - HEADER_SIZE; // Store data size only (excluding header)
    return (void *)((char *)block + HEADER_SIZE);
}

void arena_free(void *p) {
    if (p == NULL || arena_buffer == NULL) {
        return;
    }
    
    block_header_t *block = (block_header_t *)((char *)p - HEADER_SIZE);
    if ((char *)block < (char *)arena_buffer || (char *)block >= (char *)arena_buffer + arena_size) {
        return;
    }
    
    // Mark block as free
    block->size = 0;
    block->next_free = free_list;
    free_list = block;
    
    // Coalesce with neighbors
    coalesce(block);
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    block_header_t *block = (block_header_t *)((char *)p - HEADER_SIZE);
    if ((char *)block < (char *)arena_buffer || (char *)block >= (char *)arena_buffer + arena_size) {
        return NULL;
    }
    
    size_t old_size = block->size;
    size_t needed = n;
    
    // Ensure alignment
    needed = (size_t)ALIGN_UP((char *)p + n - (char *)p, _Alignof(max_align_t));
    
    // Check if we can extend into next free block
    char *end = (char *)block + HEADER_SIZE + old_size;
    block_header_t *next = (block_header_t *)end;
    
    if (next->size == 0 && (old_size + HEADER_SIZE + next->size) >= needed) {
        // Merge with next free block
        size_t additional = needed - old_size;
        if (next->size >= additional) {
            // Check if we can split remaining
            size_t remaining = next->size - additional;
            if (remaining >= MIN_BLOCK_SIZE - HEADER_SIZE) {
                // Update current block size
                block->size = needed;
                
                // Create new free block
                block_header_t *new_block = (block_header_t *)((char *)block + HEADER_SIZE + needed);
                new_block->size = remaining - HEADER_SIZE;
                new_block->next_free = free_list;
                free_list = new_block;
            } else {
                // Use entire next block
                block->size = old_size + HEADER_SIZE + next->size;
                // Update free list
                block_header_t *prev = NULL;
                block_header_t *curr = free_list;
                while (curr != next) {
                    prev = curr;
                    curr = curr->next_free;
                }
                if (prev == NULL) {
                    free_list = next->next_free;
                } else {
                    prev->next_free = next->next_free;
                }
            }
            return p;
        }
    }
    
    // Try to find a new block
    void *new_ptr = arena_alloc(n);
    if (new_ptr