#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    size_t size;
    bool used;
} BlockHeader;

static inline BlockHeader *next_block(BlockHeader *b) {
    return (BlockHeader *)((char *)b + b->size);
}

void arena_init(void *buf, size_t size) {
    BlockHeader *head = (BlockHeader *)buf;
    head->size = size;
    head->used = false;
    BlockHeader *tail = next_block(head);
    tail->size = 0; // Sentinel
    tail->used = false;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    // Round up to alignment
    size_t aligned = (n + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    BlockHeader *curr = (BlockHeader *)_Alignas(max_align_t) buf; // Uninitialized

    while (true) {
        if (!curr->used && curr->size >= aligned) {
            // Split block
            if (curr->size > aligned + sizeof(BlockHeader)) {
                BlockHeader *next = next_block(curr);
                curr->size = aligned;
                next->size = curr->size - aligned - sizeof(BlockHeader);
                next->used = false;
            }
            curr->used = true;
            return (char *)curr + sizeof(BlockHeader);
        }
        curr = next_block(curr);
        if (curr->size == 0) break;
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    
    BlockHeader *curr = (BlockHeader *)((char *)p - sizeof(BlockHeader));
    curr->used = false;
    
    // Merge with previous block if free
    BlockHeader *prev = (BlockHeader *)((char *)curr - ((BlockHeader *)((char *)curr - sizeof(BlockHeader))->size));
    if (prev->used == false && prev->size != 0) {
        curr->size += prev->size;
        curr = prev;
    }
    
    // Merge with next block if free
    BlockHeader *next = next_block(curr);
    if (next->used == false && next->size != 0) {
        curr->size += next->size;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    BlockHeader *curr = (BlockHeader *)((char *)p - sizeof(BlockHeader));
    size_t aligned = (n + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    
    // Try to grow into next block
    BlockHeader *next = next_block(curr);
    if (!next->used && curr->size + next->size >= aligned) {
        // Check if there is enough room left for a header after growing
        size_t old_size = curr->size;
        curr->size += next->size;
        
        BlockHeader *new_next = next_block(curr);
        new_next->size = old_size - aligned - sizeof(BlockHeader);
        if (new_next->size > 0) {
            new_next->used = false;
        }
        
        return p;
    }
    
    // Move if necessary
    void *new_p = arena_alloc(n);
    if (new_p != NULL) {
        size_t copy_size = n < curr->size ? n : curr->size;
        __builtin_memcpy(new_p, p, copy_size);
        arena_free(p);
    }
    return new_p;
}