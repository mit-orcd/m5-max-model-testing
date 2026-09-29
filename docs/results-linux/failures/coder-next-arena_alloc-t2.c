#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

#define ALIGNMENT _Alignof(max_align_t)

typedef struct block_header {
    size_t size;              // size of the block (including header), 0 if free
    struct block_header *next; // next block in memory order (NULL if last)
    struct block_header *prev; // previous block in memory order (NULL if first)
} block_header;

static block_header *arena_start = NULL;
static size_t arena_size = 0;

static block_header *find_free_block(size_t size) {
    block_header *block = arena_start;
    while (block && block->size == 0) {
        if (block->size == 0 && block->next) {
            // Merge with next if it's also free
            block_header *next = block->next;
            block->size = sizeof(block_header) + block->size + next->size;
            block->next = next->next;
            if (next->next) next->next->prev = block;
        } else {
            break;
        }
    }
    
    // Now find first free block that fits
    block = arena_start;
    while (block) {
        if (block->size == 0) {
            // This is a free block, check if it fits
            size_t block_end = (char*)block - (char*)arena_start + sizeof(block_header);
            size_t available = arena_size - block_end;
            if (available >= size) {
                return block;
            }
        }
        block = block->next;
    }
    return NULL;
}

static block_header *split_block(block_header *block, size_t size) {
    size_t remaining = block->size - size;
    if (remaining >= sizeof(block_header) + ALIGNMENT) {
        // Split the block
        block_header *new_block = (block_header*)((char*)block + size);
        new_block->size = 0; // Mark as free
        new_block->next = block->next;
        new_block->prev = block->prev;
        if (block->next) block->next->prev = new_block;
        if (block->prev) block->prev->next = new_block;
        else arena_start = new_block;
        block->size = size;
        block->next = new_block;
        block->prev = NULL;
        // Update prev of next block
        if (block->next) block->next->prev = block;
    }
    return block;
}

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(block_header)) {
        arena_start = NULL;
        arena_size = 0;
        return;
    }
    
    arena_start = (block_header*)buf;
    arena_size = size;
    
    // Initialize the single free block
    arena_start->size = 0;
    arena_start->next = NULL;
    arena_start->prev = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    // Align n to ALIGNMENT
    size_t aligned_n = (n + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    size_t total_size = sizeof(block_header) + aligned_n;
    
    // Find a suitable free block
    block_header *block = find_free_block(total_size);
    
    if (!block) return NULL;
    
    // Split if necessary
    block = split_block(block, total_size);
    
    // Mark as allocated
    block->size = total_size;
    block->next = NULL;
    block->prev = NULL;
    
    // Update arena_start if this is the first block
    if (!arena_start || (char*)block < (char*)arena_start) {
        arena_start = block;
    }
    
    // Return pointer to data area
    return (void*)((char*)block + sizeof(block_header));
}

void arena_free(void *p) {
    if (!p) return;
    
    block_header *block = (block_header*)((char*)p - sizeof(block_header));
    if ((char*)block < (char*)arena_start || (char*)block >= (char*)arena_start + arena_size) {
        return; // Invalid pointer
    }
    
    // Mark as free
    block->size = 0;
    
    // Merge with next block if free
    if (block->next && block->next->size == 0) {
        block_header *next = block->next;
        block->size += sizeof(block_header) + next->size;
        block->next = next->next;
        if (next->next) next->next->prev = block;
    }
    
    // Merge with prev block if free
    if (block->prev && block->prev->size == 0) {
        block_header *prev = block->prev;
        prev->size += sizeof(block_header) + block->size;
        prev->next = block->next;
        if (block->next) block->next->prev = prev;
        // Update arena_start if needed
        if ((char*)prev < (char*)arena_start) {
            arena_start = prev;
        }
    }
    
    // If this is the first block and it's free, set arena_start to it
    if ((char*)block == (char*)arena_start) {
        arena_start = block;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    block_header *block = (block_header*)((char*)p - sizeof(block_header));
    
    // Align n
    size_t aligned_n = (n + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    size_t new_total = sizeof(block_header) + aligned_n;
    size_t old_size = block->size - sizeof(block_header);
    
    // If new size is smaller or equal, just truncate
    if (new_total <= block->size) {
        // We could split here, but it's optional; just keep as is for simplicity
        return p;
    }
    
    // Check if next block is free and large enough
    block_header *next = block->next;
    if (next && next->size == 0) {
        size_t combined_size = block->size + sizeof(block_header) + next->size;
        if (combined_size >= new_total) {
            // Grow into next block
            block->size = new_total;
            next->size = 0;
            
            // Merge with next's next if possible
            if (next->next && next->next->size == 0) {
                block_header *next_next = next->next;
                next->size = 0; // Already free
                block->size += sizeof(block_header) + next_next->size;
                block->next = next_next->next;
                if (next_next->next) next_next->next->prev = block;
            } else {
                block->next = next->next;
                if (next->next) next->next->prev = block;
            }
            
            return p;
        }
    }
    
    // Otherwise allocate new block and copy
    void *new_ptr = arena_alloc(n);
    if (new_ptr) {
        size_t copy_size = old_size < n ? old_size : n;
        memcpy(new_ptr, p, copy_size);
        arena_free(p);
    }
    return new_ptr;
}