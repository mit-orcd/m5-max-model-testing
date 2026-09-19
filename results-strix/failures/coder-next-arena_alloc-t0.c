#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

typedef struct block_header {
    size_t size;           // size of the block excluding header (0 if free)
    struct block_header *next_free;  // next free block (or NULL if last)
    struct block_header *prev_free;  // prev free block (or NULL if first)
} block_header;

static block_header *arena_start = NULL;
static size_t arena_size = 0;
static block_header *free_list_head = NULL;

static void insert_free_block(block_header *block) {
    if (!free_list_head) {
        free_list_head = block;
        block->next_free = NULL;
        block->prev_free = NULL;
        return;
    }
    
    // Insert in address order
    block_header *cur = free_list_head;
    block_header *prev = NULL;
    while (cur && (void*)cur < (void*)block) {
        prev = cur;
        cur = cur->next_free;
    }
    
    if (prev) {
        prev->next_free = block;
        block->prev_free = prev;
    } else {
        free_list_head = block;
        block->prev_free = NULL;
    }
    
    block->next_free = cur;
    if (cur) {
        cur->prev_free = block;
    }
}

static void remove_free_block(block_header *block) {
    if (!block || !free_list_head) return;
    
    if (free_list_head == block) {
        free_list_head = block->next_free;
    } else {
        if (block->prev_free) {
            block->prev_free->next_free = block->next_free;
        }
    }
    
    if (block->next_free) {
        block->next_free->prev_free = block->prev_free;
    }
    
    block->next_free = NULL;
    block->prev_free = NULL;
}

void arena_init(void *buf, size_t size) {
    arena_start = (block_header*)buf;
    arena_size = ALIGN_UP(size);
    
    // Initialize free list
    free_list_head = NULL;
    
    // Create one big free block at the beginning
    if (arena_size >= sizeof(block_header) + ALIGNMENT) {
        block_header *block = arena_start;
        block->size = 0; // Mark as free
        block->next_free = NULL;
        block->prev_free = NULL;
        free_list_head = block;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    size_t total_size = ALIGN_UP(sizeof(block_header)) + ALIGN_UP(n);
    
    // First-fit search in free list
    block_header *cur = free_list_head;
    while (cur) {
        if (cur->size == 0) { // Free block
            size_t block_size = (char*)cur + sizeof(block_header);
            size_t remaining = arena_size - (block_size - (char*)arena_start);
            
            if (remaining >= total_size) {
                // Found a suitable block
                remove_free_block(cur);
                
                // Split if there's enough space for another block
                size_t used_space = sizeof(block_header) + ALIGN_UP(n);
                if (remaining >= used_space + sizeof(block_header) + ALIGNMENT) {
                    block_header *new_block = (block_header*)((char*)cur + used_space);
                    new_block->size = 0; // Mark as free
                    new_block->next_free = NULL;
                    new_block->prev_free = NULL;
                    insert_free_block(new_block);
                    
                    cur->size = ALIGN_UP(n);
                    return (void*)((char*)cur + sizeof(block_header));
                } else {
                    // Use entire block
                    cur->size = remaining - sizeof(block_header);
                    return (void*)((char*)cur + sizeof(block_header));
                }
            }
        }
        cur = cur->next_free;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    
    block_header *block = (block_header*)((char*)p - sizeof(block_header));
    
    // Mark as free
    block->size = 0;
    insert_free_block(block);
    
    // Merge with next block if free
    char *block_end = (char*)block + sizeof(block_header) + block->size;
    if (block_end < (char*)arena_start + arena_size) {
        block_header *next = (block_header*)block_end;
        if (next->size == 0) {
            remove_free_block(next);
            block->size += sizeof(block_header) + next->size;
        }
    }
    
    // Merge with previous block if free
    if (block > arena_start) {
        char *prev_end = (char*)arena_start;
        block_header *prev = arena_start;
        while (prev && (char*)prev + sizeof(block_header) + prev->size <= (char*)block) {
            if ((char*)prev + sizeof(block_header) + prev->size == (char*)block) {
                break;
            }
            prev = prev->next_free;
        }
        
        if (prev && (char*)prev + sizeof(block_header) + prev->size == (char*)block) {
            remove_free_block(prev);
            remove_free_block(block);
            prev->size += sizeof(block_header) + block->size;
            insert_free_block(prev);
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    block_header *block = (block_header*)((char*)p - sizeof(block_header));
    size_t old_size = block->size;
    
    if (n <= old_size) {
        // Shrink: do nothing, just return original pointer
        return p;
    }
    
    // Try to grow in place by merging with next block
    char *block_end = (char*)block + sizeof(block_header) + block->size;
    if (block_end < (char*)arena_start + arena_size) {
        block_header *next = (block_header*)block_end;
        if (next->size == 0) { // Next block is free
            size_t combined_size = block->size + sizeof(block_header) + next->size;
            if (combined_size >= n) {
                // Merge and update
                remove_free_block(next);
                block->size = combined_size - sizeof(block_header);
                return p;
            }
        }
    }
    
    // Need to allocate new space and copy
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) return NULL;
    
    size_t copy_size = (n < old_size) ? n : old_size;
    memcpy(new_ptr, p, copy_size);
    arena_free(p);
    
    return new_ptr;
}