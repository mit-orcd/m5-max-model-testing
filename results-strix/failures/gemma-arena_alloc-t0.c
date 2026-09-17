#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

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
    if (size < sizeof(Block) + alignof(max_align_t)) {
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

    size_t align = alignof(max_align_t);
    Block *curr = arena_state.head;

    while (curr) {
        if (curr->is_free) {
            uintptr_t data_ptr = (uintptr_t)curr + sizeof(Block);
            uintptr_t aligned_ptr = align_up(data_ptr, align);
            size_t padding = aligned_ptr - data_ptr;

            if (curr->size >= n + padding) {
                // If there is significant space left, split the block
                // We need enough space for a new Block header + alignment
                size_t total_needed = n + padding;
                if (curr->size > total_needed + sizeof(Block) + align) {
                    Block *next_block = (Block *)((uint8_t *)curr + sizeof(Block) + total_needed);
                    next_block->size = curr->size - total_needed - sizeof(Block);
                    next_block->is_free = true;
                    next_block->next = curr->next;
                    next_block->prev = curr;
                    if (curr->next) curr->next->prev = next_block;
                    curr->next = next_block;
                    curr->size = total_needed;
                }
                
                curr->is_free = false;
                // Note: In this simple implementation, padding is absorbed into the block size
                // to ensure the block remains a single contiguous unit for coalescing.
                return (void *)aligned_ptr;
            }
        }
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;

    // Find the block header. Since we align the data pointer, 
    // we must find the block that contains this pointer.
    Block *curr = arena_state.head;
    while (curr) {
        uintptr_t block_start = (uintptr_t)curr + sizeof(Block);
        uintptr_t block_end = block_start + curr->size;
        if ((uintptr_t)p >= block_start && (uintptr_t)p < block_end) {
            curr->is_free = true;

            // Coalesce with next
            if (curr->next && curr->next->is_free) {
                curr->size += sizeof(Block) + curr->next->size;
                curr->next = curr->next->next;
                if (curr->next) curr->next->prev = curr;
            }
            // Coalesce with prev
            if (curr->prev && curr->prev->is_free) {
                curr->prev->size += sizeof(Block) + curr->size;
                curr->prev->next = curr->next;
                if (curr->next) curr->next->prev = curr->prev;
            }
            return;
        }
        curr = curr->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    // Find current block
    Block *curr = arena_state.head;
    while (curr) {
        uintptr_t block_start = (uintptr_t)curr + sizeof(Block);
        uintptr_t block_end = block_start + curr->size;
        if ((uintptr_t)p >= block_start && (uintptr_t)p < block_end) break;
        curr = curr->next;
    }

    // Calculate current usable size (accounting for alignment padding)
    size_t current_usable = curr->size - ((uintptr_t)p - block_start);

    if (n <= current_usable) return p;

    // Try to grow into next block
    if (curr->next && curr->next->is_free) {
        size_t needed_from_next = n - current_usable + sizeof(Block);
        if (curr->next->size >= needed_from_next) {
            curr->size += sizeof(Block) + curr->next->size;
            curr->next = curr->next->next;
            if (curr->next) curr->next->prev = curr;
            return p;
        }
    }

    // Otherwise, allocate new and copy
    void *new_p = arena_alloc(n);
    if (new_p) {
        memcpy(new_p, p, current_usable);
        arena_free(p);
    }
    return new_p;
}