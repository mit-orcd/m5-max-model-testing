#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

#define ARENA_MAGIC 0xA110CA11U
#define ARENA_FREE  0x46524545U

typedef struct arena_block {
    uint32_t magic;
    uint32_t size;       // payload size in bytes
    struct arena_block *next;
    struct arena_block *prev;
} arena_block_t;

static arena_block_t *arena_head = NULL;
static size_t arena_capacity = 0;

static size_t align_up(size_t n, size_t align) {
    return (n + align - 1) & ~(align - 1);
}

static size_t block_total_size(size_t payload_size) {
    size_t aligned_payload = align_up(payload_size, alignof(max_align_t));
    return align_up(sizeof(arena_block_t) + aligned_payload, alignof(max_align_t));
}

static void *block_payload(arena_block_t *b) {
    return (void *)((uintptr_t)b + sizeof(arena_block_t));
}

void arena_init(void *buf, size_t size) {
    arena_head = NULL;
    arena_capacity = 0;

    if (!buf || size < block_total_size(0)) {
        return;
    }

    // Check alignment of buffer
    if ((uintptr_t)buf % alignof(max_align_t) != 0) {
        return;
    }

    arena_capacity = size;

    // Create initial free block
    size_t total = block_total_size(0);
    size_t payload_size = size - sizeof(arena_block_t);
    
    // Ensure the block fits
    if (payload_size < size - sizeof(arena_block_t)) {
        // Adjust payload size to fit exactly
        payload_size = size - sizeof(arena_block_t);
    }
    
    arena_block_t *block = (arena_block_t *)buf;
    block->magic = ARENA_FREE;
    block->size = payload_size;
    block->next = NULL;
    block->prev = NULL;
    
    arena_head = block;
}

void *arena_alloc(size_t n) {
    if (n == 0) {
        return NULL;
    }

    if (!arena_head) {
        return NULL;
    }

    // Find a free block with enough space
    arena_block_t *current = arena_head;
    while (current) {
        if (current->magic == ARENA_FREE && current->size >= n) {
            // Found a suitable block
            size_t old_size = current->size;
            size_t new_size = current->size - n;
            
            // If the remaining space is too small for a block, keep it as part of the allocation
            size_t min_free = block_total_size(0) - sizeof(arena_block_t);
            if (new_size < min_free) {
                new_size = 0;
            }
            
            if (new_size == 0) {
                // Use the entire block
                current->magic = ARENA_MAGIC;
                current->size = old_size;
            } else {
                // Split the block
                current->magic = ARENA_MAGIC;
                current->size = n;
                
                // Create a new free block for the remainder
                arena_block_t *new_block = (arena_block_t *)((uintptr_t)block_payload(current) + n);
                new_block->magic = ARENA_FREE;
                new_block->size = new_size;
                new_block->next = current->next;
                new_block->prev = current;
                
                current->next = new_block;
                if (new_block->next) {
                    new_block->next->prev = new_block;
                }
            }
            
            return block_payload(current);
        }
        current = current->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (!p) {
        return;
    }
    
    arena_block_t *block = (arena_block_t *)((uintptr_t)p - sizeof(arena_block_t));
    
    if (block->magic != ARENA_MAGIC) {
        return;
    }
    
    block->magic = ARENA_FREE;
    
    // Merge with next block if it's free
    if (block->next && block->next->magic == ARENA_FREE) {
        block->size += sizeof(arena_block_t) + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }
    
    // Merge with previous block if it's free
    if (block->prev && block->prev->magic == ARENA_FREE) {
        block->prev->size += sizeof(arena_block_t) + block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
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
    
    arena_block_t *block = (arena_block_t *)((uintptr_t)p - sizeof(arena_block_t));
    
    if (block->magic != ARENA_MAGIC) {
        return NULL;
    }
    
    size_t old_size = block->size;
    
    // Check if we can expand into the next free block
    if (block->next && block->next->magic == ARENA_FREE) {
        size_t next_size = block->next->size;
        size_t total_size = old_size + sizeof(arena_block_t) + next_size;
        
        if (total_size >= n) {
            // Expand into the next block
            if (total_size > n) {
                // Split the remaining space
                size_t new_next_size = total_size - n;
                size_t min_free = block_total_size(0) - sizeof(arena_block_t);
                
                if (new_next_size >= min_free) {
                    // Create a new free block for the remainder
                    arena_block_t *new_next = (arena_block_t *)((uintptr_t)block_payload(block) + n);
                    new_next->magic = ARENA_FREE;
                    new_next->size = new_next_size;
                    new_next->next = block->next->next;
                    new_next->prev = block;
                    
                    if (new_next->next) {
                        new_next->next->prev = new_next;
                    }
                    
                    block->next = new_next;
                } else {
                    // Use the entire remaining space
                    block->size = total_size;
                    block->next = block->next->next;
                    if (block->next) {
                        block->next->prev = block;
                    }
                }
            } else {
                // Use the entire next block
                block->size = total_size;
                block->next = block->next->next;
                if (block->next) {
                    block->next->prev = block;
                }
            }
            
            return p;
        }
    }
    
    // Allocate a new block and copy data
    void *new_p = arena_alloc(n);
    if (!new_p) {
        return NULL;
    }
    
    size_t copy_size = old_size < n ? old_size : n;
    memcpy(new_p, p, copy_size);
    
    arena_free(p);
    
    return new_p;
}