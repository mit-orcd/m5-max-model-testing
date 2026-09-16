#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

#define MAX_SIZE_T (~(size_t)0)
#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((a) - 1))

typedef struct block_header {
    size_t size;
    int free;
    struct block_header *next;
} block_header_t;

static block_header_t *free_list;
static block_header_t *first_block;
static size_t total_size;
static uint8_t *buffer_start;
static uint8_t *buffer_end;

void arena_init(void *buf, size_t size) {
    buffer_start = (uint8_t *)buf;
    buffer_end = buffer_start + size;
    total_size = size;
    
    if (size < sizeof(block_header_t)) {
        free_list = NULL;
        first_block = NULL;
        return;
    }

    // Align the start of the first block
    size_t header_size = sizeof(block_header_t);
    size_t alignment_offset = ALIGN_UP(header_size, ALIGNMENT) - header_size;
    
    if (alignment_offset + header_size > size) {
        free_list = NULL;
        first_block = NULL;
        return;
    }

    first_block = (block_header_t *)(buffer_start + alignment_offset);
    first_block->size = size - alignment_offset - header_size;
    first_block->free = 1;
    first_block->next = NULL;
    
    free_list = first_block;
}

static block_header_t *find_free_block(size_t size_needed) {
    block_header_t *curr = free_list;
    block_header_t *prev = NULL;
    
    while (curr != NULL) {
        if (curr->size >= size_needed) {
            if (prev != NULL) {
                prev->next = curr->next;
            } else {
                free_list = curr->next;
            }
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

static void add_to_free_list(block_header_t *block) {
    block->next = free_list;
    free_list = block;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    size_t aligned_n = ALIGN_UP(n, ALIGNMENT);
    if (aligned_n < sizeof(block_header_t)) {
        aligned_n = sizeof(block_header_t);
    }

    block_header_t *block = find_free_block(aligned_n);
    if (block == NULL) return NULL;

    // Check if we can split the block
    size_t remaining = block->size - aligned_n;
    if (remaining >= sizeof(block_header_t) + ALIGNMENT) {
        // Create a new free block from the remainder
        block_header_t *new_block = (block_header_t *)((uint8_t *)block + aligned_n);
        new_block->size = remaining - sizeof(block_header_t);
        new_block->free = 1;
        new_block->next = NULL;
        
        block->size = aligned_n;
        add_to_free_list(new_block);
    }

    block->free = 0;
    return (void *)(block + 1);
}

void arena_free(void *p) {
    if (p == NULL) return;
    
    block_header_t *block = (block_header_t *)((uint8_t *)p - sizeof(block_header_t));
    
    if (block < (block_header_t *)buffer_start || block >= (block_header_t *)buffer_end) {
        return;
    }
    
    block->free = 1;
    
    // Merge with next if free
    block_header_t *next = block->next;
    if (next != NULL && next->free) {
        size_t new_size = block->size + sizeof(block_header_t) + next->size;
        block->size = new_size;
        block->next = next->next;
        
        // Remove next from free list
        block_header_t *curr = free_list;
        block_header_t *prev = NULL;
        while (curr != NULL) {
            if (curr == next) {
                if (prev != NULL) {
                    prev->next = curr->next;
                } else {
                    free_list = curr->next;
                }
                break;
            }
            prev = curr;
            curr = curr->next;
        }
    }
    
    // Merge with prev if free
    // We need to traverse the linked list to find the physical predecessor
    // Since we don't have a doubly linked list, we scan for the block before this one
    block_header_t *prev = NULL;
    block_header_t *curr = first_block;
    while (curr != NULL && curr != block) {
        prev = curr;
        // Calculate address of next block based on current size
        uint8_t *next_addr = (uint8_t *)curr + curr->size + sizeof(block_header_t);
        if (next_addr > (uint8_t *)buffer_end) break;
        curr = (block_header_t *)next_addr;
    }
    
    if (prev != NULL && prev->free) {
        // Merge prev into block
        size_t new_size = prev->size + sizeof(block_header_t) + block->size;
        prev->size = new_size;
        prev->next = block->next;
        
        // Remove block from free list
        block_header_t *curr_list = free_list;
        block_header_t *prev_list = NULL;
        while (curr_list != NULL) {
            if (curr_list == block) {
                if (prev_list != NULL) {
                    prev_list->next = curr_list->next;
                } else {
                    free_list = curr_list->next;
                }
                break;
            }
            prev_list = curr_list;
            curr_list = curr_list->next;
        }
        
        block = prev;
    }
    
    add_to_free_list(block);
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    block_header_t *block = (block_header_t *)((uint8_t *)p - sizeof(block_header_t));
    size_t old_size = block->size;
    size_t aligned_n = ALIGN_UP(n, ALIGNMENT);
    if (aligned_n < sizeof(block_header_t)) {
        aligned_n = sizeof(block_header_t);
    }
    
    if (old_size >= aligned_n) {
        // Check if we can shrink
        size_t remaining = old_size - aligned_n;
        if (remaining >= sizeof(block_header_t) + ALIGNMENT) {
            block_header_t *new_block = (block_header_t *)((uint8_t *)block + aligned_n);
            new_block->size = remaining - sizeof(block_header_t);
            new_block->free = 1;
            new_block->next = block->next;
            
            block->size = aligned_n;
            add_to_free_list(new_block);
        }
        return p;
    }
    
    // Try to grow into next block
    block_header_t *next = block->next;
    if (next != NULL && next->free) {
        size_t combined_size = block->size + sizeof(block_header_t) + next->size;
        if (combined_size >= aligned_n) {
            // Merge
            block->size = combined_size;
            
            // Remove next from free list
            block_header_t *curr = free_list;
            block_header_t *prev = NULL;
            while (curr != NULL) {
                if (curr == next) {
                    if (prev != NULL) {
                        prev->next = curr->next;
                    } else {
                        free_list = curr->next;
                    }
                    break;
                }
                prev = curr;
                curr = curr->next;
            }
            
            // Copy data if size increased
            if (aligned_n > old_size) {
                // Data preserved up to old_size
                // No copy needed as we are expanding in place
            }
            
            return p;
        }
    }
    
    // Allocate new block and copy
    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    
    size_t copy_size = (old_size < n) ? old_size : n;
    if (copy_size > 0) {
        memcpy(new_p, p, copy_size);
    }
    
    arena_free(p);
    return new_p;
}

// Helper for memcpy since we can't use standard library functions in this context
static void memcpy(void *dst, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) *d++ = *s++;
}