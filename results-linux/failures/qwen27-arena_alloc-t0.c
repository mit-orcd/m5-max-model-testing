#include <stdalign.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#define ARENA_MAGIC 0xA5E5A5E5u

typedef struct arena_block {
    size_t size;
    int free;
    struct arena_block *next;
    struct arena_block *prev;
} arena_block_t;

typedef struct arena_state {
    uint32_t magic;
    char *base;
    size_t total_size;
    arena_block_t *head;
} arena_state_t;

static arena_state_t *g_arena = NULL;

static size_t align_up(size_t n, size_t align) {
    return (n + align - 1) & ~(align - 1);
}

static size_t block_overhead(void) {
    return align_up(sizeof(arena_block_t), alignof(max_align_t));
}

static arena_block_t *block_from_ptr(void *p) {
    return (arena_block_t *)p;
}

static void *block_data(arena_block_t *b) {
    return (char *)b + block_overhead();
}

static size_t block_payload_size(arena_block_t *b) {
    return b->size - block_overhead();
}

static void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(arena_state_t) + block_overhead()) {
        g_arena = NULL;
        return;
    }

    arena_state_t *st = (arena_state_t *)buf;
    st->magic = ARENA_MAGIC;
    st->base = (char *)buf;
    st->total_size = size;

    size_t state_size = align_up(sizeof(arena_state_t), alignof(max_align_t));
    char *block_start = (char *)buf + state_size;
    size_t block_area_size = size - state_size;

    if (block_area_size < block_overhead()) {
        g_arena = NULL;
        return;
    }

    arena_block_t *head = (arena_block_t *)block_start;
    head->size = block_area_size;
    head->free = 1;
    head->next = NULL;
    head->prev = NULL;

    st->head = head;
    g_arena = st;
}

static arena_state_t *get_arena(void) {
    if (!g_arena || g_arena->magic != ARENA_MAGIC) {
        return NULL;
    }
    return g_arena;
}

static void insert_block_after(arena_block_t *prev, arena_block_t *new_block) {
    new_block->prev = prev;
    new_block->next = prev->next;
    if (prev->next) {
        prev->next->prev = new_block;
    }
    prev->next = new_block;
}

static void remove_block(arena_block_t *b) {
    if (b->prev) {
        b->prev->next = b->next;
    }
    if (b->next) {
        b->next->prev = b->prev;
    }
}

static void merge_with_next(arena_block_t *b) {
    if (b->next && b->next->free) {
        arena_block_t *next = b->next;
        b->size += next->size;
        remove_block(next);
        b->next = next->next;
        if (b->next) {
            b->next->prev = b;
        }
    }
}

static void merge_with_prev(arena_block_t *b) {
    if (b->prev && b->prev->free) {
        arena_block_t *prev = b->prev;
        prev->size += b->size;
        remove_block(b);
        prev->next = b->next;
        if (prev->next) {
            prev->next->prev = prev;
        }
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) {
        return NULL;
    }

    arena_state_t *st = get_arena();
    if (!st) {
        return NULL;
    }

    size_t aligned_n = align_up(n, alignof(max_align_t));
    size_t needed = block_overhead() + aligned_n;

    arena_block_t *b = st->head;
    while (b) {
        if (b->free && b->size >= needed) {
            if (b->size > needed) {
                size_t split_size = b->size - needed;
                arena_block_t *new_block = (arena_block_t *)((char *)b + needed);
                new_block->size = split_size;
                new_block->free = 1;
                new_block->prev = b;
                new_block->next = b->next;
                if (b->next) {
                    b->next->prev = new_block;
                }
                b->next = new_block;
                b->size = needed;
            }
            b->free = 0;
            return block_data(b);
        }
        b = b->next;
    }

    return NULL;
}

void arena_free(void *p) {
    if (!p) {
        return;
    }

    arena_state_t *st = get_arena();
    if (!st) {
        return;
    }

    arena_block_t *b = block_from_ptr(p);
    if (b < st->head || (char *)b >= st->base + st->total_size) {
        return;
    }

    b->free = 1;
    merge_with_next(b);
    merge_with_prev(b);
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        if (p) {
            arena_free(p);
        }
        return NULL;
    }

    if (!p) {
        return arena_alloc(n);
    }

    arena_state_t *st = get_arena();
    if (!st) {
        return NULL;
    }

    arena_block_t *b = block_from_ptr(p);
    if (b < st->head || (char *)b >= st->base + st->total_size) {
        return NULL;
    }

    size_t aligned_n = align_up(n, alignof(max_align_t));
    size_t needed = block_overhead() + aligned_n;

    if (b->size >= needed) {
        if (b->size > needed) {
            size_t split_size = b->size - needed;
            arena_block_t *new_block = (arena_block_t *)((char *)b + needed);
            new_block->size = split_size;
            new_block->free = 1;
            new_block->prev = b;
            new_block->next = b->next;
            if (b->next) {
                b->next->prev = new_block;
            }
            b->next = new_block;
            b->size = needed;
        }
        return p;
    }

    if (b->next && b->next->free && b->size + b->next->size >= needed) {
        size_t total = b->size + b->next->size;
        arena_block_t *next = b->next;
        remove_block(next);
        b->size = total;
        b->next = next->next;
        if (b->next) {
            b->next->prev = b;
        }

        if (b->size > needed) {
            size_t split_size = b->size - needed;
            arena_block_t *new_block = (arena_block_t *)((char *)b + needed);
            new_block->size = split_size;
            new_block->free = 1;
            new_block->prev = b;
            new_block->next = b->next;
            if (b->next) {
                b->next->prev = new_block;
            }
            b->next = new_block;
            b->size = needed;
        }
        return p;
    }

    void *new_p = arena_alloc(n);
    if (!new_p) {
        return NULL;
    }

    size_t old_size = block_payload_size(b);
    size_t copy_size = old_size < n ? old_size : n;
    memcpy(new_p, p, copy_size);
    arena_free(p);
    return new_p;
}