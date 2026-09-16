#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct Block {
    size_t size;
    bool is_free;
    struct Block *next;
    struct Block *prev;
} Block;

static struct {
    void *buffer;
    size_t capacity;
    Block *head;
} arena_state;

static uintptr_t align_up(uintptr_t addr, size_t align) {
    return (addr + (align - 1)) & ~(align - 1);
}

void arena_init(void *buf, size_t size) {
    arena_state.buffer = buf;
    arena_state.capacity = size;
    
    if (size < sizeof(Block) + sizeof(max_align_t)) {
        arena_state.head = NULL;
        return;
    }

    arena_state.head = (Block *)buf;
    arena_state.head->size = size - sizeof(Block);
    arena_state.head->is_free = true;
    arena_state.head->next = NULL;
    arena_state.head->prev = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;

    size_t required_size = n;
    // Ensure the payload itself is aligned for max_align_t
    // The block header is placed before the aligned payload
    
    Block *curr = arena_state.head;
    while (curr) {
        if (curr->is_free) {
            uintptr_t payload_ptr = (uintptr_t)curr + sizeof(Block);
            uintptr_t aligned_payload = align_up(payload_ptr, _Alignof(max_align_t));
            size_t padding = aligned_payload - payload_ptr;
            size_t total_needed = sizeof(Block) + padding + required_size;

            // We need to account for the fact that the header stays at its pos,
            // but the usable space starts after padding.
            // To simplify: we ensure (curr + sizeof(Block)) is aligned.
            // Since Block is likely aligned, we just check if we can fit it.
            
            // Re-evaluating: Let's force the block to start such that 
            // (Block*) + sizeof(Block) is aligned.
            // This requires the Block itself to be aligned.
        }
        curr = curr->next;
    }
    // Refined approach: 
    // The header is at 'curr'. The payload starts at 'curr + 1'.
    // We must ensure 'curr + 1' is aligned.
    return NULL; // Placeholder for logic below
}