#include <stddef.h>
#include <stdalign.h>
#include <string.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

typedef struct block_header {
    size_t size;
    int is_free;
    struct block_header *next;
    struct block_header *prev;
} block_header_t;

static block_header_t *first_block;
static size_t total_size;
static size_t current_offset;

void arena_init(void *buf, size_t size) {
    if (size < sizeof(block_header_t) + ALIGNMENT) {
        first_block = NULL;
        total_size = 0;
        return;
    }
    
    // Align the start of the usable area
    char *aligned_buf = (char *)buf;
    size_t padding = ALIGN_UP((size_t)aligned_buf) - (size_t)aligned_buf;
    
    if (padding >= size) {
        first_block = NULL;
        total_size = 0;
        return;
    }
    
    char *header_start = aligned_buf + padding;
    block_header_t *header = (block_header_t *)header_start;
    
    size_t usable_size = size - padding - sizeof(block_header_t);
    if (usable_size < ALIGNMENT) {
        first_block = NULL;
        total_size = 0;
        return;
    }
    
    header->size = usable_size;
    header->is_free = 1;
    header->next = NULL;
    header->prev = NULL;
    
    first_block = header;
    total_size = size;
    current_offset = padding + sizeof(block_header_t);
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    size_t aligned_n = ALIGN_UP(n);
    if (aligned_n < sizeof(block_header_t)) {
        aligned_n = sizeof(block_header_t);
    }
    
    block_header_t *curr = first_block;
    while (curr != NULL) {
        if (curr->is_free && curr->size >= aligned_n) {
            // Found a suitable block
            size_t remaining = curr->size - aligned_n;
            
            if (remaining >= sizeof(block_header_t) + ALIGNMENT) {
                // Split the block
                block_header_t *new_block = (block_header_t *)((char *)curr + aligned_n);
                new_block->size = remaining - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = curr->next;
                new_block->prev = curr;
                
                if (curr->next != NULL) {
                    curr->next->prev = new_block;
                }
                
                curr->size = aligned_n;
                curr->next = new_block;
                
                return (char *)curr + sizeof(block_header_t);
            } else {
                // Use the whole block
                curr->is_free = 0;
                return (char *)curr + sizeof(block_header_t);
            }
        }
        curr = curr->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    
    block_header_t *block = (block_header_t *)((char *)p - sizeof(block_header_t));
    block->is_free = 1;
    
    // Merge with next if free
    if (block->next != NULL && block->next->is_free) {
        block->size += sizeof(block_header_t) + block->next->size;
        block->next = block->next->next;
        if (block->next != NULL) {
            block->next->prev = block;
        }
    }
    
    // Merge with prev if free
    if (block->prev != NULL && block->prev->is_free) {
        block->prev->size += sizeof(block_header_t) + block->size;
        block->prev->next = block->next;
        if (block->next != NULL) {
            block->next->prev = block->prev;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    block_header_t *block = (block_header_t *)((char *)p - sizeof(block_header_t));
    size_t old_size = block->size;
    size_t aligned_n = ALIGN_UP(n);
    if (aligned_n < sizeof(block_header_t)) {
        aligned_n = sizeof(block_header_t);
    }
    
    if (aligned_n <= old_size) {
        // Can fit in current block, no need to move
        return p;
    }
    
    // Check if next block is free and large enough to grow into
    if (block->next != NULL && block->next->is_free) {
        size_t next_size = block->next->size;
        size_t required = aligned_n - old_size;
        
        if (next_size >= required) {
            // Grow into next block
            block->size = aligned_n;
            
            size_t remaining = next_size - required;
            if (remaining >= sizeof(block_header_t) + ALIGNMENT) {
                block_header_t *new_block = (block_header_t *)((char *)block + aligned_n);
                new_block->size = remaining - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = block->next->next;
                new_block->prev = block;
                
                if (block->next->next != NULL) {
                    block->next->next->prev = new_block;
                }
                
                block->next = new_block;
            } else {
                // Consume the next block entirely
                block->next = block->next->next;
                if (block->next != NULL) {
                    block->next->prev = block;
                }
            }
            
            return p;
        }
    }
    
    // Need to allocate new block and copy
    void *new_p = arena_alloc(n);
    if (new_p == NULL) {
        return NULL;
    }
    
    size_t copy_size = (old_size < aligned_n) ? old_size : aligned_n;
    memcpy(new_p, p, copy_size);
    arena_free(p);
    
    return new_p;
}