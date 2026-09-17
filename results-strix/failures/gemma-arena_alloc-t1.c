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

static void *g_arena_buf = NULL;
static size_t g_arena_size = 0;
static Block *g_head = NULL;

static uintptr_t align_up(uintptr_t addr, size_t align) {
    return (addr + (align - 1)) & ~(align - 1);
}

void arena_init(void *buf, size_t size) {
    g_arena_buf = buf;
    g_arena_size = size;
    if (size < sizeof(Block)) {
        g_head = NULL;
        return;
    }
    g_head = (Block *)buf;
    g_head->size = size - sizeof(Block);
    g_head->is_free = true;
    g_head->next = NULL;
    g_head->prev = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0 || !g_arena_buf) return NULL;

    size_t align = _Alignof(max_align_t);
    Block *curr = g_head;
    while (curr) {
        if (curr->is_free) {
            uintptr_t data_ptr = (uintptr_t)curr + sizeof(Block);
            uintptr_t aligned_ptr = align_up(data_ptr, align);
            size_t padding = aligned_ptr - data_ptr;
            
            if (curr->size >= n + padding) {
                // If there is significant space left, split the block
                // We need enough space for a new Block header + at least 1 byte
                size_t total_needed = n + padding;
                if (curr->size >= total_needed + sizeof(Block) + 1) {
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
                return (void *)aligned_ptr;
            }
        }
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p || !g_arena_buf) return;

    // To find the block, we must account for the alignment padding.
    // Since we don't store padding, we scan from head.
    // However, the requirement implies p is the start of the usable area.
    // We find the block by checking which block's range contains p.
    Block *curr = g_head;
    while (curr) {
        uintptr_t start = (uintptr_t)curr + sizeof(Block);
        uintptr_t end = start + curr->size;
        if ((uintptr_t)p >= start && (uintptr_t)p < end) {
            curr->is_free = true;
            
            // Merge with next
            if (curr->next && curr->next->is_free) {
                curr->size += sizeof(Block) + curr->next->size;
                curr->next = curr->next->next;
                if (curr->next) curr->next->prev = curr;
            }
            // Merge with prev
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

    // Find the block containing p
    Block *curr = g_head;
    while (curr) {
        uintptr_t start = (uintptr_t)curr + sizeof(Block);
        uintptr_t end = start + curr->size;
        if ((uintptr_t)p >= start && (uintptr_t)p < end) {
            // Calculate actual usable size including padding
            size_t padding = (uintptr_t)p - start;
            size_t current_usable_size = curr->size - padding;

            if (n <= current_usable_size) return p;

            // Try to grow into next block
            if (curr->next && curr->next->is_free) {
                size_t needed_extra = n - current_usable_size + padding;
                // The next block's size must be enough to cover the header + needed extra
                if (curr->next->size >= needed_extra + sizeof(Block)) {
                    // This is simplified; actual alignment makes "growing" tricky.
                    // For a robust implementation, we check if the next block 
                    // can be absorbed without shifting the current p.
                    // Given the constraints, we attempt a simple merge:
                    size_t combined_size = curr->size + sizeof(Block) + curr->next->size;
                    // Re-check if the combined block can accommodate n with current padding
                    if (combined_size - padding >= n) {
                        curr->size = combined_size;
                        curr->next = curr->next->next;
                        if (curr->next) curr->next->prev = curr;
                        return p;
                    }
                }
            }

            // If cannot grow, move
            void *new_p = arena_alloc(n);
            if (new_p) {
                size_t copy_size = (current_usable_size < n) ? current_usable_size : n;
                memcpy(new_p, p, copy_size);
                arena_free(p);
            }
            return new_p;
        }
        curr = curr->next;
    }
    return NULL;
}