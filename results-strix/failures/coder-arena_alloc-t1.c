#include <stddef.h>
#include <stdint.h>
#include <string.h>

static void *arena_buf;
static size_t arena_size;
static size_t arena_used;

typedef struct free_block {
    size_t size;
    struct free_block *next;
} free_block_t;

static free_block_t *free_list;

void arena_init(void *buf, size_t size) {
    arena_buf = buf;
    arena_size = size;
    arena_used = 0;
    free_list = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    // Align to max_align_t
    size_t align = _Alignof(max_align_t);
    n = (n + align - 1) & ~(align - 1);
    
    // Try to find a free block
    free_block_t **prev = &free_list;
    free_block_t *current = free_list;
    
    while (current) {
        if (current->size >= n) {
            // Found a block that fits
            *prev = current->next;
            
            // If the block is significantly larger, split it
            if (current->size - n >= sizeof(free_block_t)) {
                free_block_t *new_free = (free_block_t*)((char*)current + n);
                new_free->size = current->size - n;
                new_free->next = NULL;
                free_list = new_free;
            }
            
            return (void*)current;
        }
        prev = &current->next;
        current = current->next;
    }
    
    // No free block found, try to allocate from the buffer
    size_t offset = (arena_used + align - 1) & ~(align - 1);
    if (offset + n > arena_size) {
        return NULL;
    }
    
    arena_used = offset + n;
    return (void*)((char*)arena_buf + offset);
}

void arena_free(void *p) {
    if (p == NULL) return;
    
    // Find the block that contains p
    free_block_t *block = (free_block_t*)p;
    
    // Add to free list
    block->next = free_list;
    free_list = block;
    
    // Try to merge with adjacent free blocks
    free_block_t *current = free_list;
    free_block_t **prev = &free_list;
    
    while (current) {
        free_block_t *next = current->next;
        char *end = (char*)current + current->size;
        
        // Check if next block is adjacent and free
        if (next && (char*)next == end) {
            current->size += next->size;
            current->next = next->next;
        } else {
            prev = &current->next;
            current = current->next;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    // Align to max_align_t
    size_t align = _Alignof(max_align_t);
    n = (n + align - 1) & ~(align - 1);
    
    // Try to grow in place
    free_block_t *block = (free_block_t*)p;
    size_t current_size = block->size;
    
    // Check if the next block is free and can accommodate the new size
    char *end = (char*)block + current_size;
    free_block_t *next_block = (free_block_t*)end;
    
    if (next_block->next == NULL && next_block->size > 0) {
        // Check if we can merge and grow
        size_t total_free = next_block->size;
        if (current_size + total_free >= n) {
            // Can grow in place
            block->size = n;
            return p;
        }
    }
    
    // Need to allocate new space and copy
    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    
    memcpy(new_p, p, (n < current_size) ? n : current_size);
    arena_free(p);
    return new_p;
}