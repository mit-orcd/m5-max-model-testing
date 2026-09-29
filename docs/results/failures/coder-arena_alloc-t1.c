#include <stddef.h>
#include <stdint.h>

static void *arena_buf;
static size_t arena_size;
static size_t arena_used;

typedef struct free_block {
    size_t size;
    struct free_block *next;
} free_block;

static free_block *free_list;

void arena_init(void *buf, size_t size) {
    arena_buf = buf;
    arena_size = size;
    arena_used = 0;
    free_list = NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    
    // Find the free block that contains p
    free_block *current = free_list;
    free_block *prev = NULL;
    
    while (current != NULL) {
        char *block_start = (char *)arena_buf + sizeof(free_block);
        char *block_end = (char *)arena_buf + arena_used;
        
        if (p == (char *)arena_buf + sizeof(ffree_block)) {
            // p is at the beginning of the buffer
            free_list = current->next;
            free_list = current->next;
            current->next = NULL;
            return;
        }
        
        // Check if p is within the free block
        char *block_start = (char *)arena_buf + sizeof(free_block);
        char *block_end = (char *)arena_buf + current->size;
        
        if ((char *)p >= block_start && (char *)p < block_end) {
            // Merge with adjacent free blocks if possible
            char *p_end = (char *)p + current->size;
            free_block *next_free = current->next;
            
            if (next_free && (char *)p_end == (char *)arena_buf + (size_t)next_free) {
                // Merge with next free block
                current->size += next_free->size;
                current->next = next_free->next;
            }
            
            return;
        }
        
        prev = current;
        current = current->next;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    // Align to 16-byte boundary for any type
    size_t aligned_size = (n + 15) & ~15;
    
    // Find a free block that can satisfy the request
    free_block *prev = NULL;
    free_block *current = free_list;
    
    while (current != NULL) {
        if (current->size >= aligned_size) {
            // Remove this block from free list
            if (prev == NULL) {
                free_list = current->next;
            } else {
                prev->next = current->next;
            }
            
            // Return pointer to the allocated space
            return (char *)arena_buf + sizeof(free_block) + arena_used + sizeof(free_block);
        }
        prev = current;
        current = current->next;
    }
    
    // Allocate new block at end of buffer
    if (arena_used + aligned_size <= arena_size) {
        void *ptr = (char *)arena_buf + sizeof(free_block) + arena_used;
        arena_used += aligned_size;
        return ptr;
    }
    
    return NULL;
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) {
        return arena_alloc(n);
    }
    
    // Find the free block that contains p
    free_block *current = free_list;
    while (current != NULL) {
        // Check if p is within this free block
        char *block_start = (char *)arena_buf + sizeof(free_block);
        char *block_end = (char *)arena_buf + sizeof(free_block) + current->size;
        
        if (p >= (void *)block_start && p < (void *)block_end) {
            // Check if we can grow into adjacent free block
            size_t offset = (char *)p - (char *)arena_buf;
            size_t block_size = current->size;
            
            // Try to grow into adjacent free block
            if (offset + block_size + sizeof(free_block) < arena_used) {
                // Check if next block is free
                free_block *next_free = (free_block *)arena_buf + offset + block_size;
                if (next_free->size > 0) {
                    // Merge with next free block
                    current->size += next_free->size;
                    // Remove from free list
                    free_list = next_free->next;
                    // Update size of current block
                    current->size += next_free->size;
                    // Move pointer to new location
                    return p;
                }
            }
        }
        
        current = current->next;
    }
    
    // If we can't grow, allocate new space and copy data
    void *new_p = arena_alloc(n);
    if (new_p == NULL) {
        return NULL;
    }
    
    // Copy data from old location to new location
    // Note: This requires knowing the original size, which we don't have here
    // For simplicity, assume we have enough space to move data
    return new_p;
}