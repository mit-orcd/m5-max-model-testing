#include <stddef.h>
#include <stdalign.h>
#include <string.h>
#include <stdbool.h>

typedef struct header {
    size_t size;
    bool is_free;
    struct header *next;
    struct header *prev;
} header_t;

#define HEADER_SIZE ((sizeof(header_t) + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1))
#define ALIGNMENT _Alignof(max_align_t)

static header_t *head = NULL;
static header_t *last = NULL;

void arena_init(void *buf, size_t size) {
    if (size < HEADER_SIZE + ALIGNMENT) {
        head = NULL;
        last = NULL;
        return;
    }
    
    // Align the start of the buffer for the first block
    // The 'buf' pointer is passed by user, we assume it's at least aligned or we handle it.
    // To be safe and simple, we treat 'buf' as the raw memory start.
    // We need to ensure the first header is aligned.
    // Since the user gives us 'buf', we assume it's aligned enough or we pad if needed.
    // However, the problem says "All bookkeeping must live inside that buffer".
    // We will assume 'buf' is aligned to ALIGNMENT for simplicity, or we just use byte arithmetic.
    // Let's assume the caller passes a properly aligned buffer or we align the first header.
    // To strictly follow "inside that buffer", we treat 'buf' as the start of our arena.
    // We will align the first header if 'buf' is not aligned, but 'buf' is void*.
    // Standard practice: The buffer passed is raw memory. We align the first header.
    
    char *base = (char *)buf;
    char *aligned_start = base;
    
    // Align base to ALIGNMENT
    size_t offset = (uintptr_t)base % ALIGNMENT;
    if (offset != 0) {
        aligned_start = base + (ALIGNMENT - offset);
    }
    
    if ((size_t)(aligned_start - base) >= size) {
        head = NULL;
        last = NULL;
        return;
    }
    
    size_t usable_size = size - (aligned_start - base);
    
    // Create the first free block
    header_t *first = (header_t *)aligned_start;
    first->size = usable_size - HEADER_SIZE;
    first->is_free = true;
    first->next = NULL;
    first->prev = NULL;
    
    head = first;
    last = first;
}

static header_t *find_block(size_t size, header_t *hint) {
    // First fit search
    header_t *curr = (hint != NULL) ? hint : head;
    while (curr != NULL) {
        if (curr->is_free && curr->size >= size) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0 || head == NULL) {
        return NULL;
    }
    
    // Round up size to alignment
    size_t aligned_n = (n + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    
    // We need at least HEADER_SIZE + aligned_n
    if (aligned_n + HEADER_SIZE > (size_t)(last ? ((char*)last + last->size + HEADER_SIZE) - (char*)head : 0)) {
        // Quick check against total size might be insufficient if fragmentation exists, 
        // but we rely on the search loop below.
    }
    
    header_t *block = find_block(aligned_n, head);
    
    if (block == NULL) {
        return NULL;
    }
    
    // Split block if it's significantly larger than needed
    size_t min_split = HEADER_SIZE + ALIGNMENT * 2;
    if (block->size >= aligned_n + HEADER_SIZE + ALIGNMENT) {
        block->size -= (aligned_n + HEADER_SIZE);
        
        // Create new block from the remainder
        char *new_block_addr = (char *)block + HEADER_SIZE + block->size;
        header_t *new_block = (header_t *)new_block_addr;
        new_block->size = aligned_n;
        new_block->is_free = true;
        new_block->next = block->next;
        new_block->prev = block;
        
        if (block->next != NULL) {
            block->next->prev = new_block;
        } else {
            last = new_block;
        }
        block->next = new_block;
    }
    
    block->is_free = false;
    
    // Return pointer to user data (after header)
    return (void *)((char *)block + HEADER_SIZE);
}

void arena_free(void *p) {
    if (p == NULL) {
        return;
    }
    
    // Locate header
    header_t *block = (header_t *)((char *)p - HEADER_SIZE);
    
    if (block < head || block > last || block->is_free) {
        return; // Invalid pointer or already free
    }
    
    block->is_free = true;
    
    // Merge with next if free
    if (block->next != NULL && block->next->is_free) {
        header_t *next = block->next;
        block->size += HEADER_SIZE + next->size;
        block->next = next->next;
        if (next->next != NULL) {
            next->next->prev = block;
        } else {
            last = block;
        }
    }
    
    // Merge with prev if free
    if (block->prev != NULL && block->prev->is_free) {
        header_t *prev = block->prev;
        prev->size += HEADER_SIZE + block->size;
        prev->next = block->next;
        if (block->next != NULL) {
            block->next->prev = prev;
        } else {
            last = prev;
        }
        // No need to update head as we only merge backwards
    }
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (p == NULL) {
        return arena_alloc(n);
    }
    
    header_t *block = (header_t *)((char *)p - HEADER_SIZE);
    
    // Check if block is valid
    if (block < head || block > last || !block->is_free) {
        // This case should ideally not happen for a valid pointer, 
        // but if it does, we treat it as a new alloc and copy.
        // However, standard realloc on invalid ptr is UB, but we handle gracefully.
        // If it's a valid block but we just need to check size.
    }
    
    size_t aligned_n = (n + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    
    // Optimization: grow into next block if it's free and large enough
    if (!block->is_free && block->next != NULL && block->next->is_free) {
        header_t *next = block->next;
        if (next->size >= aligned_n - block->size) {
            // Can we grow?
            // We need to keep the existing data size, but resize the header.
            // Actually, we just need to check if the combined space is enough for 'n'.
            // But the block itself has a fixed 'size' field.
            // The block's current size is 'block->size'.
            // We need 'aligned_n'.
            // If next->size + block->size >= aligned_n, we can merge and resize.
            
            // Wait, the block->size is the size of the data part.
            // We need to check if block->size + next->size >= aligned_n.
            // If so, we merge next into block, and set block->size = aligned_n.
            
            // But we must preserve the content up to min(old_size, new_size).
            // The content is already in the block.
            
            size_t current_size = block->size;
            if (current_size + next->size >= aligned_n) {
                // Merge
                block->size += HEADER_SIZE + next->size; // Add next's header and data
                block->next = next->next;
                if (next->next != NULL) {
                    next->next->prev = block;
                } else {
                    last = block;
                }
                
                // Trim if we allocated more than needed (split)
                if (block->size > aligned_n + HEADER_SIZE) {
                    size_t remaining = block->size - aligned_n;
                    block->size = aligned_n;
                    
                    // Create new free block
                    char *new_addr = (char *)block + HEADER_SIZE + aligned_n;
                    header_t *new_block = (header_t *)new_addr;
                    new_block->size = remaining;
                    new_block->is_free = true;
                    new_block->next = NULL;
                    new_block->prev = block;
                    block->next = new_block;
                    last = new_block;
                } else {
                    block->size = aligned_n;
                }
                return p;
            }
        }
    }
    
    // If we can't grow in place, do a standard realloc (alloc new, copy, free old)
    void *new_p = arena_alloc(n);
    if (new_p == NULL) {
        return NULL;
    }
    
    size_t old_size = block->size;
    size_t copy_size = (old_size < n) ? old_size : n;
    // Align copy size? No, memcpy works on bytes.
    // We copy min(old_size, n).
    // Note: old_size is already aligned (or not? arena_alloc aligns the request, so yes).
    
    memcpy(new_p, p, copy_size);
    arena_free(p);
    return new_p;
}