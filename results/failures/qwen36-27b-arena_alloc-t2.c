#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct arena_block {
    size_t size;
    int free;
} arena_block_t;

static void *arena_buf = NULL;
static size_t arena_buf_size = 0;
static arena_block_t *arena_first = NULL;

void arena_init(void *buf, size_t size) {
    if (buf == NULL || size < sizeof(arena_block_t)) {
        arena_buf = NULL;
        arena_buf_size = 0;
        arena_first = NULL;
        return;
    }
    arena_buf = buf;
    arena_buf_size = size;
    arena_first = (arena_block_t *)buf;
    arena_first->size = size - sizeof(arena_block_t);
    arena_first->free = 1;
}

static inline arena_block_t *next_block(arena_block_t *b) {
    arena_block_t *next = (arena_block_t *)((char *)b + sizeof(arena_block_t) + b->size);
    if ((char *)next >= (char *)arena_buf + arena_buf_size) {
        return NULL;
    }
    return next;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    if (arena_first == NULL) return NULL;

    size_t aligned_n = (n + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1);
    if (aligned_n < n) return NULL; // overflow check

    arena_block_t *current = arena_first;
    while (current != NULL) {
        if (current->free && current->size >= aligned_n) {
            size_t remaining = current->size - aligned_n;
            if (remaining > 0 && remaining >= sizeof(arena_block_t)) {
                arena_block_t *new_block = (arena_block_t *)((char *)current + sizeof(arena_block_t) + aligned_n);
                new_block->size = remaining;
                new_block->free = 1;
                current->size = aligned_n;
            }
            current->free = 0;
            return (char *)current + sizeof(arena_block_t);
        }
        current = next_block(current);
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL || arena_first == NULL) return;
    
    arena_block_t *b = (arena_block_t *)((char *)p - sizeof(arena_block_t));
    if ((char *)b < (char *)arena_buf || (char *)b + sizeof(arena_block_t) + b->size > (char *)arena_buf + arena_buf_size) {
        return; // Invalid pointer
    }
    
    b->free = 1;
    
    // Merge with next block
    arena_block_t *next = next_block(b);
    if (next != NULL && next->free) {
        b->size += sizeof(arena_block_t) + next->size;
    }
    
    // Merge with previous block
    arena_block_t *prev = arena_first;
    arena_block_t *prev_prev = NULL;
    while (prev != b) {
        prev_prev = prev;
        prev = next_block(prev);
        if (prev == NULL) return; // Should not happen if p is valid
    }
    
    if (prev_prev != NULL && prev_prev->free) {
        prev_prev->size += sizeof(arena_block_t) + b->size;
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
    
    if (arena_first == NULL) return NULL;
    
    arena_block_t *b = (arena_block_t *)((char *)p - sizeof(arena_block_t));
    if ((char *)b < (char *)arena_buf || (char *)b + sizeof(arena_block_t) + b->size > (char *)arena_buf + arena_buf_size) {
        return NULL; // Invalid pointer
    }
    
    size_t aligned_n = (n + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1);
    if (aligned_n < n) return NULL; // overflow check
    
    // Check if we can expand into the next free block
    arena_block_t *next = next_block(b);
    if (next != NULL && next->free && b->size + sizeof(arena_block_t) + next->size >= aligned_n) {
        // Merge with next
        b->size += sizeof(arena_block_t) + next->size;
        
        // Check if we need to split
        size_t remaining = b->size - aligned_n;
        if (remaining > 0 && remaining >= sizeof(arena_block_t)) {
            arena_block_t *new_block = (arena_block_t *)((char *)b + sizeof(arena_block_t) + aligned_n);
            new_block->size = remaining;
            new_block->free = 1;
            b->size = aligned_n;
        }
        b->free = 0;
        return p;
    }
    
    // Cannot expand in place, allocate new and copy
    size_t old_size = b->size;
    size_t copy_size = n < old_size ? n : old_size;
    
    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    
    memcpy(new_p, p, copy_size);
    arena_free(p);
    
    return new_p;
}