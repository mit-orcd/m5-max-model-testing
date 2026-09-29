#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ARENA_ALIGN (sizeof(max_align_t))

#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((a) - 1))

typedef struct arena_block {
    size_t size;
    int free;
    struct arena_block *next;
} arena_block_t;

typedef struct arena_state {
    char *start;
    size_t total_size;
    arena_block_t *first;
} arena_state_t;

static arena_state_t g_arena;

static void *align_ptr(void *p, size_t align) {
    uintptr_t addr = (uintptr_t)p;
    uintptr_t aligned = (addr + align - 1) & ~(align - 1);
    return (void *)aligned;
}

static size_t block_overhead(void) {
    return sizeof(arena_block_t);
}

static size_t min_block_size(void) {
    size_t overhead = block_overhead();
    size_t aligned = ALIGN_UP(overhead, ARENA_ALIGN);
    if (aligned < overhead) aligned = overhead;
    return aligned;
}

static arena_block_t *block_from_ptr(void *p) {
    return (arena_block_t *)((char *)p - block_overhead());
}

static void *block_data(arena_block_t *b) {
    return (char *)b + block_overhead();
}

static void arena_init(void *buf, size_t size) {
    g_arena.start = (char *)buf;
    g_arena.total_size = size;
    g_arena.first = NULL;

    if (size < min_block_size()) {
        return;
    }

    arena_block_t *b = (arena_block_t *)align_ptr(buf, ARENA_ALIGN);
    b->size = size - ((char *)b - (char *)buf);
    b->free = 1;
    b->next = NULL;
    g_arena.first = b;
}

static void insert_block(arena_block_t *prev, arena_block_t *new_block) {
    new_block->next = prev->next;
    prev->next = new_block;
}

static void remove_block(arena_block_t *prev, arena_block_t *to_remove) {
    prev->next = to_remove->next;
}

static void merge_with_next(arena_block_t *b) {
    if (b->next && b->next->free) {
        b->size += b->next->size;
        b->next = b->next->next;
    }
}

static void *arena_alloc(size_t n) {
    if (n == 0) return NULL;

    size_t needed = ALIGN_UP(n, ARENA_ALIGN);
    size_t total_needed = needed + block_overhead();

    if (total_needed < min_block_size()) {
        total_needed = min_block_size();
    }

    arena_block_t *prev = NULL;
    arena_block_t *cur = g_arena.first;

    while (cur) {
        if (cur->free && cur->size >= total_needed) {
            if (cur->size > total_needed) {
                arena_block_t *new_block = (arena_block_t *)((char *)cur + total_needed);
                new_block->size = cur->size - total_needed;
                new_block->free = 1;
                new_block->next = cur->next;
                cur->size = total_needed;
                cur->next = new_block;
            }
            cur->free = 0;
            return block_data(cur);
        }
        prev = cur;
        cur = cur->next;
    }

    return NULL;
}

static void arena_free(void *p) {
    if (p == NULL) return;

    arena_block_t *b = block_from_ptr(p);
    b->free = 1;

    arena_block_t *prev = NULL;
    arena_block_t *cur = g_arena.first;

    while (cur) {
        if (cur == b) break;
        prev = cur;
        cur = cur->next;
    }

    if (cur != b) return;

    if (prev && prev->free) {
        prev->size += b->size;
        prev->next = b->next;
        b = prev;
    }

    merge_with_next(b);
}

static void *arena_realloc(void *p, size_t n) {
    if (p == NULL) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    arena_block_t *b = block_from_ptr(p);
    size_t old_size = b->size - block_overhead();
    size_t new_needed = ALIGN_UP(n, ARENA_ALIGN);
    size_t total_needed = new_needed + block_overhead();

    if (total_needed < min_block_size()) {
        total_needed = min_block_size();
    }

    if (b->size >= total_needed) {
        if (b->size > total_needed) {
            arena_block_t *new_block = (arena_block_t *)((char *)b + total_needed);
            new_block->size = b->size - total_needed;
            new_block->free = 1;
            new_block->next = b->next;
            b->size = total_needed;
            b->next = new_block;
        }
        return p;
    }

    if (b->next && b->next->free && b->size + b->next->size >= total_needed) {
        size_t combined = b->size + b->next->size;
        arena_block_t *next_next = b->next->next;
        b->size = total_needed;
        if (combined > total_needed) {
            arena_block_t *new_block = (arena_block_t *)((char *)b + total_needed);
            new_block->size = combined - total_needed;
            new_block->free = 1;
            new_block->next = next_next;
            b->next = new_block;
        } else {
            b->next = next_next;
        }
        return p;
    }

    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;

    size_t copy_size = old_size < n ? old_size : n;
    memcpy(new_p, p, copy_size);
    arena_free(p);
    return new_p;
}