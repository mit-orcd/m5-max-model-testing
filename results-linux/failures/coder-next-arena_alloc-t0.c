#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

typedef struct block_header {
    size_t size;           // size of the block excluding header (0 if free)
    struct block_header *next;  // next block in memory order
    struct block_header *prev;  // previous block in memory order
} block_header;

static void *arena_buffer = NULL;
static size_t arena_size = 0;
static block_header *first_block = NULL;

#define ALIGN_UP(p, a) (((uintptr_t)(p) + (a) - 1) & ~((a) - 1))
#define HEADER_SIZE ALIGN_UP(sizeof(block_header), _Alignof(max_align_t))

void arena_init(void *buf, size_t size) {
    if (!buf || size < HEADER_SIZE) {
        arena_buffer = NULL;
        arena_size = 0;
        first_block = NULL;
        return;
    }
    
    arena_buffer = buf;
    arena_size = size;
    
    // Create initial single free block
    block_header *initial = (block_header *)buf;
    initial->size = 0;  // mark as free
    initial->next = NULL;
    initial->prev = NULL;
    first_block = initial;
}

void *arena_alloc(size_t n) {
    if (!arena_buffer || n == 0) {
        return NULL;
    }
    
    // Align requested size
    size_t aligned_n = ALIGN_UP(n, _Alignof(max_align_t));
    
    // First-fit search for a free block large enough
    block_header *block = first_block;
    while (block) {
        if (block->size == 0) {  // free block
            size_t block_size = (block->next ? (char *)block->next - (char *)block : arena_size) - HEADER_SIZE;
            if (block_size >= aligned_n) {
                // Found a suitable block
                size_t remaining = block_size - aligned_n;
                
                // Split if remaining space is enough for another header + minimal data
                if (remaining >= HEADER_SIZE + _Alignof(max_align_t)) {
                    block_header *new_block = (block_header *)((char *)block + HEADER_SIZE + aligned_n);
                    new_block->size = 0;  // mark as free
                    new_block->next = block->next;
                    new_block->prev = block->prev;
                    if (new_block->next) {
                        new_block->next->prev = new_block;
                    }
                    if (new_block->prev) {
                        new_block->prev->next = new_block;
                    } else {
                        first_block = new_block;
                    }
                    
                    // Update current block to be allocated
                    block->size = aligned_n;
                    block->next = new_block;
                    block->prev = new_block->prev;
                    
                    return (void *)((char *)block + HEADER_SIZE);
                } else {
                    // Use entire block
                    block->size = aligned_n;
                    // Remove from free list (mark as allocated)
                    if (block->prev) {
                        block->prev->next = block->next;
                    } else {
                        first_block = block->next;
                    }
                    if (block->next) {
                        block->next->prev = block->prev;
                    }
                    return (void *)((char *)block + HEADER_SIZE);
                }
            }
        }
        block = block->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (!p || !arena_buffer) {
        return;
    }
    
    block_header *block = (block_header *)((char *)p - HEADER_SIZE);
    
    // Mark as free
    block->size = 0;
    
    // Insert into free list (maintain memory order)
    block_header *prev = NULL;
    block_header *curr = first_block;
    
    // Find insertion point (before first block with higher address)
    while (curr && (char *)curr < (char *)block) {
        prev = curr;
        curr = curr->next;
    }
    
    // Insert block between prev and curr
    block->prev = prev;
    block->next = curr;
    if (prev) {
        prev->next = block;
    } else {
        first_block = block;
    }
    if (curr) {
        curr->prev = block;
    }
    
    // Coalesce with neighbors
    // Coalesce with next block
    if (block->next && block->next->size == 0) {
        block_header *next = block->next;
        block->size = (char *)next + HEADER_SIZE + next->size - (char *)block - HEADER_SIZE;
        block->next = next->next;
        if (next->next) {
            next->next->prev = block;
        }
    }
    
    // Coalesce with previous block
    if (block->prev && block->prev->size == 0) {
        block_header *prev = block->prev;
        prev->size = (char *)block + HEADER_SIZE + block->size - (char *)prev - HEADER_SIZE;
        prev->next = block->next;
        if (block->next) {
            block->next->prev = prev;
        }
        // Update first_block if needed
        if (first_block == block) {
            first_block = prev;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!arena_buffer) {
        return NULL;
    }
    
    if (!p) {
        return arena_alloc(n);
    }
    
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    block_header *block = (block_header *)((char *)p - HEADER_SIZE);
    size_t old_size = block->size;
    
    // If same or smaller size, just return
    if (n <= old_size) {
        return p;
    }
    
    size_t new_size = ALIGN_UP(n, _Alignof(max_align_t));
    
    // Try to grow into next block if it's free and large enough
    if (block->next && block->next->size == 0) {
        size_t next_block_size = (block->next->next ? (char *)block->next->next - (char *)block->next : arena_size) - HEADER_SIZE;
        if (old_size + next_block_size >= new_size) {
            // Grow into next block
            size_t needed = new_size - old_size;
            size_t remaining = next_block_size - needed;
            
            // Update current block size
            block->size = new_size;
            
            // Remove next block from free list
            block_header *next = block->next;
            block->next = next->next;
            if (next->next) {
                next->next->prev = block;
            }
            
            // Create new free block if remaining space is sufficient
            if (remaining >= HEADER_SIZE + _Alignof(max_align_t)) {
                block_header *new_block = (block_header *)((char *)block + HEADER_SIZE + new_size);
                new_block->size = 0;
                new_block->next = next->next;
                new_block->prev = block;
                if (new_block->next) {
                    new_block->next->prev = new_block;
                }
                if (new_block->prev) {
                    new_block->prev->next = new_block;
                } else {
                    first_block = new_block;
                }
            }
            
            return p;
        }
    }
    
    // Fall back to allocating new memory and copying
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) {
        return NULL;
    }
    
    memcpy(new_ptr, p, old_size);
    arena_free(p);
    return new_ptr;
}