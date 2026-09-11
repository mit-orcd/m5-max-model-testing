#include <stddef.h>
#include <stddef.h>
#include <string.h>

typedef struct block_header {
    size_t size;
    int is_free;
    struct block_header *next;
    struct block_header *prev;
} block_header_t;

static struct {
    void *buf;
    size_t size;
    block_header_t *head;
} g_arena;

static size_t align_up(size_t size, size_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

void arena_init(void *buf, size_t size) {
    g_arena.buf = buf;
    g_arena.size = size;
    g_arena.head = (block_header_t *)buf;
    g_arena.head->size = size - sizeof(block_header_t);
    g_arena.head->is_free = 1;
    g_arena.head->next = NULL;
    g_arena.head->prev = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t required_size = align_up(n, _Alignof(max_align_t));
    block_header_t *curr = g_arena.head;

    while (curr) {
        if (curr->is_free && curr->size >= required_size) {
            // Check if we can split
            size_t min_split = sizeof(block_header_t) + _Alignof(max_align_t);
            if (curr->size >= required_size + min_split) {
                block_header_t *new_block = (block_header_t *)((char *)curr + sizeof(block_header_t) + required_size);
                new_block->size = curr->size - required_size - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = curr->next;
                new_block->prev = curr;
                if (curr->next) curr->next->prev = new_block;
                curr->next = new_block;
                curr->size = required_size;
            }
            curr->is_free = 0;
            return (void *)((char *)curr + sizeof(block_header_t));
        }
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    block_header_t *curr = (block_header_t *)((char *)p - sizeof(block_header_t));
    curr->is_free = 1;

    // Merge with next
    if (curr->next && curr->next->is_free) {
        curr->size += sizeof(block_header_t) + curr->next->size;
        curr->next = curr->next->next;
        if (curr->next) curr->next->prev = curr;
    }
    // Merge with prev
    if (curr->prev && curr->prev->is_free) {
        curr->prev->size += sizeof(block_header_t) + curr->size;
        curr->prev->next = curr->next;
        if (curr->next) curr->next->prev = curr->prev;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    block_header_t *curr = (block_header_t *)((char *)p - sizeof(block_header_t));
    size_t required_size = align_up(n, _Alignof(max_align_t));

    if (curr->size >= required_size) {
        // Try to grow into next block
        if (curr->next && curr->next->is_free) {
            size_t potential_size = curr->size + sizeof(block_header_t) + curr->next->size;
            if (potential_size >= required_size) {
                // It's large enough to absorb the next block
                size_t old_size = curr->size;
                // If we can consume the whole next block
                curr->size = potential_size;
                curr->next = curr->next->next;
                if (curr->next) curr->next->prev = curr;
                // If we still have excess, we don't split yet to keep it simple, 
                // but the logic allows for future splits.
                // However, to satisfy "return p unchanged", we just absorb.
                // If the absorbed size is much larger, we might need to split to be efficient,
                // but the requirement says "grow into that neighbour".
                // Let's check if we should split the absorbed part if it's still too big.
                // To keep p valid and size updated:
                // We only absorb if the requirement is met.
                // If after absorbing we have enough for a new header + alignment, split.
                size_t min_split = sizeof(block_header_t) + _Alignof(max_align_t);
                if (curr->size >= required_size + min_split) {
                    // This part is tricky. If we split, we might lose the "return p" advantage.
                    // But if we don't split, we waste space. The requirement says:
                    // "when the block immediately after p is free and large enough it must grow into that neighbour"
                }
                // To strictly follow: absorb and return.
                return p;
            }
        }
        // If not growing, we still check if current is enough (already checked)
        return p;
    }

    // If current is too small, move
    void *new_p = arena_alloc(n);
    if (new_p) {
        size_t old_size = curr->size;
        size_t copy_size = (old_size < n) ? old_size : n;
        memcpy(new_p, p, copy_size);
        arena_free(p);
    }
    return new_p;
}