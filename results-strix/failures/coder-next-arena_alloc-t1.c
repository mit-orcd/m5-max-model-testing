```c
#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

typedef struct block_header {
    size_t size;              // size of data area (excluding header)
    int is_free;              // 1 if block is free, 0 otherwise
    struct block_header *next_free;  // next free block in free list (only valid if is_free)
    struct block_header *prev_free;  // prev free block in free list (only valid if is_free)
} block_header;

static void *arena_buffer = NULL;
static size_t arena_size = 0;
static block_header *free_list_head = NULL;

static size_t align_up(size_t x, size_t align) {
    return (x + align - 1) & ~(align - 1);
}

void arena_init(void *buf, size_t size) {
    arena_buffer = buf;
    arena_size = size;
    free_list_head = NULL;
    
    if (size < sizeof(block_header)) return;
    
    // Create initial free block
    block_header *initial = (block_header *)buf;
    initial->size = size - sizeof(block_header);
    initial->is_free = 1;
    initial->next_free = NULL;
    initial->prev_free = NULL;
    
    free_list_head = initial;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    // Align request to max_align_t
    size_t aligned_n = align_up(n, _Alignof(max_align_t));
    if (aligned_n > arena_size) return NULL;
    
    // First-fit search in free list
    block_header *prev = NULL;
    block_header *curr = free_list_head;
    
    while (curr != NULL) {
        if (curr->is_free && curr->size >= aligned_n) {
            // Found suitable block
            size_t remaining = curr->size - aligned_n;
            
            // Remove from free list
            if (curr->prev_free) curr->prev_free->next_free = curr->next_free;
            if (curr->next_free) curr->next_free->prev_free = curr->prev_free;
            if (free_list_head == curr) free_list_head = curr->next_free;
            
            // Split if remaining space is big enough for a header + minimal payload
            if (remaining >= sizeof(block_header) + _Alignof(max_align_t)) {
                block_header *new_block = (block_header *)((char *)curr + sizeof(block_header) + aligned_n);
                new_block->size = remaining - sizeof(block_header);
                new_block->is_free = 1;
                new_block->next_free = free_list_head;
                new_block->prev_free = NULL;
                if (free_list_head) free_list_head->prev_free = new_block;
                free_list_head = new_block;
                
                curr->size = aligned_n;
            }
            
            curr->is_free = 0;
            return (void *)((char *)curr + sizeof(block_header));
        }
        
        prev = curr;
        curr = curr->next_free;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    
    block_header *block = (block_header *)((char *)p - sizeof(block_header));
    
    if (!block->is_free) {
        block->is_free = 1;
        
        // Merge with next block if free
        char *end = (char *)block + sizeof(block_header) + block->size;
        block_header *next = (block_header *)end;
        
        if (next < (block_header *)((char *)arena_buffer + arena_size) && 
            next->is_free) {
            // Remove next from free list
            if (next->prev_free) next->prev_free->next_free = next->next_free;
            if (next->next_free) next->next_free->prev_free = next->prev_free;
            if (free_list_head == next) free_list_head = next->next_free;
            
            // Merge
            block->size += sizeof(block_header) + next->size;
        }
        
        // Merge with previous block if free
        // Find previous block by scanning from buffer start
        block_header *curr = (block_header *)arena_buffer;
        while ((char *)curr < (char *)block) {
            block_header *next_header = (block_header *)((char *)curr + sizeof(block_header) + curr->size);
            if (next_header == block) {
                // Found the previous block
                if (curr->is_free) {
                    // Remove curr from free list
                    if (curr->prev_free) curr->prev_free->next_free = curr->next_free;
                    if (curr->next_free) curr->next_free->prev_free = curr->prev_free;
                    if (free_list_head == curr) free_list_head = curr->next_free;
                    
                    // Merge
                    curr->size += sizeof(block_header) + block->size;
                    
                    // Now curr is the merged block; reinsert into free list if needed
                    curr->next_free = free_list_head;
                    if (free_list_head) free_list_head->prev_free = curr;
                    curr->prev_free = NULL;
                    free_list_head = curr;
                }
                break;
            }
            curr = next_header;
        }
        
        // If we didn't merge with prev, insert into free list
        if (block->is_free && 
            ((char *)block + sizeof(block_header) + block->size > (char *)arena_buffer + arena_size ||
             (block != (block_header *)((char *)arena_buffer + arena_size - sizeof(block_header) - block->size) && 
              block != free_list_head))) {
            // Insert at head of free list
            block->next_free = free_list_head;
            block->prev_free = NULL;
            if (free_list_head) free_list_head->prev_free = block;
            free_list_head = block;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    block_header *block = (block_header *)((char *)p - sizeof(block_header));
    size_t old_size = block->size;
    size_t new_size = align_up(n, _Alignof(max_align_t));
    
    if (new_size <= old_size) {
        // Shrink: do nothing, just return pointer
        return p;
    }
    
    // Try to grow in place
    char *end = (char *)block + sizeof(block_header) + block->size;
    block_header *next = (block_header *)end;
    
    // Check if next block is free and large enough
    if (next < (block_header *)((char *)arena_buffer + arena_size) &&
        next->is_free &&
        block->size + sizeof(block_header) + next->size >= new_size) {
        
        // Remove next from free list
        if (next->prev_free) next->prev_free->next_free = next->next_free;
        if (next->next_free) next->next_free->prev_free = next->prev_free;
        if (free_list_head == next) free_list_head = next->next_free;
        
        // Merge next into current block
        size_t remaining = block->size + sizeof(block_header) + next->size - new_size - sizeof(block_header);
        
        // Update current block size
        block->size = new_size;
        
        // Create new free block if remaining space is sufficient
        if (remaining >= sizeof(block_header) + _Alignof(max_align_t)) {
            block_header *new_block = (block_header *)((char *)block + sizeof(block_header) + new_size);
            new_block->size = remaining - sizeof(block_header);
            new_block->is_free = 1;
            new_block->next_free = free_list_head;
            new_block->prev_free = NULL;
            if (free_list_head) free_list_head->prev_free = new_block;
            free_list_head = new_block;
        }
        
        return p;
    }
    
    // Need to allocate new block and copy
    void *new_ptr = arena_alloc(n);
    if (new_ptr == NULL) return NULL;
    
    // Copy data
    size_t copy_size = (old_size < new_size) ? old_size : new_size;
    _Alignas(max_align_t) char temp[1];
    (void)temp;