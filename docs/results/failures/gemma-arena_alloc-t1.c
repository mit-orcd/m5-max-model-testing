#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct Block {
    size_t size;
    int is_free;
    struct Block *next;
} Block;

static struct {
    void *buf;
    size_t size;
    Block *head;
} g_arena;

static uintptr_t align_up(uintptr_t addr, size_t align) {
    return (addr + (align - 1)) & ~(align - 1);
}

void arena_init(void *buf, size_t size) {
    g_arena.buf = buf;
    g_arena.size = size;
    g_arena.head = (Block *)buf;
    g_arena.head->size = size - sizeof(Block);
    g_arena.head->is_free = 1;
    g_arena.head->next = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t align = _Alignof(max_align_t);
    
    Block *curr = g_arena.head;
    while (curr) {
        if (curr->is_free) {
            uintptr_t block_start = (uintptr_t)curr + sizeof(Block);
            uintptr_t aligned_start = align_up(block_start, align);
            size_t padding = aligned_start - block_start;

            if (curr->size >= n + padding) {
                // If there is padding to satisfy alignment, we can't easily 
                // split if the padding is part of the block size.
                // However, we can treat the block as having the padding 
                // as unusable space or move the block pointer logic.
                // Simplest: ensure block size accounts for the header + alignment.
            }
        }
        curr = curr->next;
    }

    // Standard First-Fit with alignment adjustment
    curr = g_arena.head;
    while (curr) {
        if (curr->is_free) {
            uintptr_t base = (uintptr_t)curr + sizeof(Block);
            uintptr_t aligned = align_up(base, align);
            size_t needed = n + (aligned - base);

            if (curr->size >= needed) {
                // If we have significant excess, split. 
                // But we must ensure the next header can fit.
                size_t total_needed = n; 
                // To keep bookkeeping simple, we just use the block.
                // But we need to handle the offset.
                
                // Let's refine: The block's capacity is curr->size.
                // The user gets (aligned - base) offset + n.
                // We check if we can split.
                
                // If padding is 0, it's a standard split.
                if (aligned == base) {
                    if (curr->size >= n + sizeof(Block)) {
                        Block *next_b = (Block *)((char *)curr + sizeof(Block) + n);
                        next_b->size = curr->size - n - sizeof(Block);
                        next_b->is_free = 1;
                        next_b->next = curr->next;
                        curr->next = next_b;
                        curr->size = n;
                    } else {
                        curr->is_free = 0;
                    }
                    return (void *)base;
                } else {
                    // If there's padding, we can't easily split the "header" 
                    // because the header is at the start. 
                    // For this implementation, we'll ensure the block is large enough.
                    // To support alignment, we'll use the block as is.
                    // Wait, the simplest way to support alignment in a fixed buffer:
                    // The header is at the start. We offset the 'user' pointer.
                }
            }
        }
        curr = curr->next;
    }
    return NULL;
}