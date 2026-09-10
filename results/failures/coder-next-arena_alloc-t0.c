#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

#define BLOCK_MAGIC 0xDEADBEEF

typedef struct block_header {
    size_t size;           // size of the block excluding header (0 if free)
    size_t prev_size;      // size of previous block (only if free)
    struct block_header *next;
    struct block_header *prev;
    uint32_t magic;
} block_header_t;

static block_header_t *arena_start = NULL;
static block_header_t *arena_end = NULL;
static block_header_t *free_list = NULL;

static size_t align_up(size_t size, size_t align) {
    return (size + align - 1) & ~(align - 1);
}

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(block_header_t)) {
        arena_start = NULL;
        arena_end = NULL;
        free_list = NULL;
        return;
    }
    
    // Align buffer start to max_align_t
    uintptr_t aligned_start = (uintptr_t)buf;
    size_t offset = align_up(aligned_start, _Alignof(max_align_t)) - aligned_start;
    if (offset + sizeof(block_header_t) > size) {
        arena_start = NULL;
        arena_end = NULL;
        free_list = NULL;
        return;
    }
    
    char *aligned_buf = (char *)buf + offset;
    size -= offset;
    
    arena_start = (block_header_t *)aligned_buf;
    arena_end = (block_header_t *)(aligned_buf + size - sizeof(block_header_t));
    
    // Create one big free block
    arena_start->size = 0; // mark as free
    arena_start->prev_size = 0;
    arena_start->next = NULL;
    arena_start->prev = NULL;
    arena_start->magic = BLOCK_MAGIC;
    
    free_list = arena_start;
}

void *arena_alloc(size_t n) {
    if (!arena_start || n == 0) {
        return NULL;
    }
    
    // Align requested size
    size_t aligned_n = align_up(n, _Alignof(max_align_t));
    if (aligned_n == 0) {
        return NULL; // overflow
    }
    
    // Find first free block that fits
    block_header_t *block = free_list;
    while (block) {
        if (block->size == 0 && block->prev_size + sizeof(block_header_t) + aligned_n <= 
            ((char *)block - (char *)arena_start) + ((char *)arena_end - (char *)arena_start + sizeof(block_header_t))) {
            // Check actual available space in this block
            size_t block_start = (char *)block - (char *)arena_start;
            size_t block_end = block_start + sizeof(block_header_t) + block->prev_size;
            size_t available = ((char *)arena_end - (char *)arena_start + sizeof(block_header_t)) - block_end;
            if (available >= aligned_n) {
                break;
            }
        }
        block = block->next;
    }
    
    if (!block) {
        return NULL;
    }
    
    // Split block if there's enough space left
    size_t block_start = (char *)block - (char *)arena_start;
    size_t block_end = block_start + sizeof(block_header_t) + block->prev_size;
    size_t available = ((char *)arena_end - (char *)arena_start + sizeof(block_header_t)) - block_end;
    
    if (available >= aligned_n + sizeof(block_header_t) + _Alignof(max_align_t)) {
        // Split the block
        block_header_t *new_block = (block_header_t *)((char *)block + sizeof(block_header_t) + block->prev_size + aligned_n);
        new_block->size = 0; // free
        new_block->prev_size = aligned_n;
        new_block->next = block->next;
        new_block->prev = block->prev;
        new_block->magic = BLOCK_MAGIC;
        
        if (block->next) {
            block->next->prev = new_block;
        }
        if (block->prev) {
            block->prev->next = new_block;
        } else {
            free_list = new_block;
        }
        
        block->next = new_block;
        block->prev = NULL;
    }
    
    // Mark block as allocated
    block->size = aligned_n;
    block->prev_size = 0;
    
    // Remove from free list
    if (block->prev) {
        block->prev->next = block->next;
    } else {
        free_list = block->next;
    }
    if (block->next) {
        block->next->prev = block->prev;
    }
    block->next = NULL;
    block->prev = NULL;
    
    return (void *)(block + 1);
}

void arena_free(void *p) {
    if (!p || !arena_start) {
        return;
    }
    
    block_header_t *block = (block_header_t *)p - 1;
    if (block->magic != BLOCK_MAGIC) {
        return;
    }
    
    // Mark as free
    size_t block_start = (char *)block - (char *)arena_start;
    size_t block_end = block_start + sizeof(block_header_t) + block->size;
    size_t arena_size = (char *)arena_end - (char *)arena_start + sizeof(block_header_t);
    
    block->size = 0;
    block->prev_size = 0;
    
    // Find previous block
    block_header_t *prev_block = NULL;
    if (block_start > 0) {
        prev_block = arena_start;
        while (prev_block && (char *)(prev_block + 1) + prev_block->size <= (char *)block) {
            if ((char *)(prev_block + 1) + prev_block->size == (char *)block) {
                break;
            }
            prev_block = prev_block->next;
        }
    }
    
    // Find next block
    block_header_t *next_block = NULL;
    if (block_end < arena_size) {
        next_block = (block_header_t *)((char *)block + sizeof(block_header_t) + block->size);
        if ((char *)next_block > (char *)arena_end) {
            next_block = NULL;
        }
    }
    
    // Merge with previous block
    if (prev_block && prev_block->size == 0) {
        // Merge with previous
        prev_block->size += sizeof(block_header_t) + block->size;
        prev_block->next = block->next;
        if (block->next) {
            block->next->prev = prev_block;
        }
        block = prev_block;
    }
    
    // Merge with next block
    if (next_block && next_block->size == 0) {
        block->size += sizeof(next_block) + next_block->size;
        block->next = next_block->next;
        if (next_block->next) {
            next_block->next->prev = block;
        }
    }
    
    // Insert into free list (at beginning for simplicity)
    block->next = free_list;
    block->prev = NULL;
    if (free_list) {
        free_list->prev = block;
    }
    free_list = block;
}

void *arena_realloc(void *p, size_t n) {
    if (!p) {
        return arena_alloc(n);
    }
    if (!arena_start) {
        return NULL;
    }
    
    block_header_t *block = (block_header_t *)p - 1;
    if (block->magic != BLOCK_MAGIC) {
        return NULL;
    }
    
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    size_t aligned_n = align_up(n, _Alignof(max_align_t));
    if (aligned_n == 0) {
        return NULL; // overflow
    }
    
    // Check if we can grow in place
    if (block->size >= aligned_n) {
        return p;
    }
    
    // Check if next block is free and large enough
    block_header_t *next_block = block->next;
    if (next_block && next_block->size == 0) {
        size_t combined_size = block->size + sizeof(block_header_t) + next_block->size;
        if (combined_size >= aligned_n) {
            // Merge with next block
            block->size = combined_size - sizeof(block_header_t);
            block->next = next_block->next;
            if (next_block->next) {
                next_block->next->prev = block;
            }
            return p;
        }
    }
    
    // Need to allocate new space and copy
    void *new_ptr = arena_alloc(aligned_n);
    if (!new_ptr) {
        return NULL;
    }
    
    memcpy(new_ptr, p, block->size < aligned_n ? block->size : aligned_n);
    arena_free(p);
    
    return new_ptr;
}