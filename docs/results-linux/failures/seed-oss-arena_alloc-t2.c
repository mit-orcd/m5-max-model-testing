#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define ALIGN _Alignof(max_align_t)

typedef struct arena_block {
    size_t size;
    bool is_free;
    struct arena_block *next;
} arena_block;

typedef struct {
    size_t total_size;
    arena_block *free_list;
} arena_state;

static arena_state *current_arena = NULL;

static inline size_t align_up(size_t x) {
    return (x + ALIGN - 1) / ALIGN * ALIGN;
}

void arena_init(void *buf, size_t size) {
    if (size < sizeof(arena_state)) {
        current_arena = NULL;
        return;
    }
    current_arena = (arena_state *)buf;
    current_arena->total_size = size;
    current_arena->free_list = NULL;

    char *block_start = (char *)buf + sizeof(arena_state);
    size_t block_size = size - sizeof(arena_state);
    if (block_size > 0) {
        arena_block *initial = (arena_block *)block_start;
        initial->size = block_size;
        initial->is_free = true;
        initial->next = NULL;
        current_arena->free_list = initial;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0 || !current_arena) return NULL;

    arena_block *prev = NULL, *curr = current_arena->free_list;
    while (curr) {
        char *block_start = (char *)curr;
        size_t user_start = align_up((size_t)(block_start + sizeof(arena_block)));
        size_t required = user_start - (size_t)block_start + n;

        if (curr->size >= required) {
            arena_block *new_free = NULL;
            if (curr->size > required) {
                new_free = (arena_block *)((char *)curr + required);
                new_free->size = curr->size - required;
                new_free->is_free = true;
                new_free->next = curr->next;
            }
            (prev ? prev->next : current_arena->free_list) = new_free;
            curr->is_free = false;
            curr->size = required;
            return (void *)user_start;
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p || !current_arena) return;

    char *p_char = (char *)p;
    arena_block *block = NULL;
    for (size_t pad = 0; pad < ALIGN; pad++) {
        char *cand = p_char - (sizeof(arena_block) + pad);
        if (cand < (char *)current_arena || cand >= (char *)current_arena + current_arena->total_size) continue;
        size_t user_start = align_up((size_t)(cand + sizeof(arena_block)));
        if ((char *)user_start == p_char) { block = (arena_block *)cand; break; }
    }
    if (!block || block->is_free) return;

    block->is_free = true;

    // Coalesce next
    char *block_end = (char *)block + block->size;
    if (block_end < (char *)current_arena + current_arena->total_size) {
        arena_block *next = (arena_block *)block_end;
        if (next->is_free) {
            block->size += next->size;
            arena_block **prev_next = &current_arena->free_list;
            while (*prev_next && *prev_next != next) prev_next = &(*prev_next)->next;
            if (*prev_next == next) *prev_next = next->next;
        }
    }

    // Coalesce previous
    char *block_start = (char *)block;
    arena_block *prev_block = NULL, *curr = (arena_block *)((char *)current_arena + sizeof(arena_state));
    while (curr) {
        char *curr_end = (char *)curr + curr->size;
        if (curr_end == block_start) { prev_block = curr; break; }
        curr = (arena_block *)curr_end;
    }
    if (prev_block && prev_block->is_free) {
        prev_block->size += block->size;
        arena_block **prev_next = &current_arena->free_list;
        while (*prev_next && *prev_next != block) prev_next = &(*prev_next)->next;
        if (*prev_next == block) *prev_next = block->next;
        block = prev_block;
    }

    // Add to free list
    block->next = current_arena->free_list;
    current_arena->free_list = block;
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    if (!current_arena) return NULL;

    char *p_char = (char *)p;
    arena_block *block = NULL;
    for (size_t pad = 0; pad < ALIGN; pad++) {
        char *cand = p_char - (sizeof(arena_block) + pad);
        if (cand < (char *)current_arena || cand >= (char *)current_arena + current_arena->total_size) continue;
        size_t user_start = align_up((size_t)(cand + sizeof(arena_block)));
        if ((char *)user_start == p_char) { block = (arena_block *)cand; break; }
    }
    if (!block || block->is_free) return NULL;

    size_t u_offset = p_char - (char *)block;
    size_t old_size = block->size - u_offset;
    size_t required = align_up((size_t)((char *)block + sizeof(arena_block))) - (size_t)block + n;

    // Try to grow into next block
    char *next_start = (char *)block + block->size;
    if (next_start < (char *)current_arena + current_arena->total_size) {
        arena_block *next = (arena_block *)next_start;
        if (next->is_free && next->size >= (required - block->size)) {
            size_t total = block->size + next->size;
            if (total > required) {
                arena_block *new_free = (arena_block *)((char *)block + required);
                new_free->size = total - required;
                new_free->is_free = true;
                new_free->next = current_arena->free_list;
                current_arena->free_list = new_free;
                block->size = required;
            } else {
                block->size = total;
                arena_block **prev_next = &current_arena->free_list;
                while (*prev_next && *prev_next != next) prev_next = &(*prev_next)->next;
                if (*prev_next == next) *prev_next = next->next;
            }
            block->is_free = false;
            return p;
        }
    }

    // Allocate new and copy
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) return NULL;
    size_t copy = (old_size < n) ? old_size : n;
    memcpy(new_ptr, p, copy);
    arena_free(p);
    return new_ptr;
}