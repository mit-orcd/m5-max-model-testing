```c
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ARENA_MAGIC 0xA5E5A5E5u

typedef struct arena_header {
    uint32_t magic;
    uint32_t size;      /* payload size */
    uint32_t prev_size; /* size of previous block's payload, 0 if first */
    uint32_t free;      /* 1 if free, 0 if allocated */
    struct arena_header *next;
    struct arena_header *prev;
} arena_header_t;

typedef struct arena_state {
    uint32_t magic;
    uint32_t size;
    arena_header_t *first;
    arena_header_t *last;
} arena_state_t;

static arena_state_t *g_arena = NULL;

static size_t align_up(size_t n, size_t align) {
    return (n + align - 1) & ~(align - 1);
}

static size_t header_size(void) {
    return align_up(sizeof(arena_header_t), _Alignof(max_align_t));
}

static void *payload_from_header(arena_header_t *h) {
    return (void *)((char *)h + header_size());
}

static arena_header_t *header_from_payload(void *p) {
    return (arena_header_t *)((char *)p - header_size());
}

static void insert_free_list(arena_header_t *h) {
    h->free = 1;
    h->next = g_arena->first;
    h->prev = NULL;
    if (g_arena->first) {
        g_arena->first->prev = h;
    }
    g_arena->first = h;
    if (!g_arena->last || (char *)h > (char *)g_arena->last) {
        g_arena->last = h;
    }
}

static void remove_free_list(arena_header_t *h) {
    if (h->prev) {
        h->prev->next = h->next;
    } else {
        g_arena->first = h->next;
    }
    if (h->next) {
        h->next->prev = h->prev;
    }
    h->prev = NULL;
    h->next = NULL;
    h->free = 0;
}

static arena_header_t *next_header(arena_header_t *h) {
    size_t total = header_size() + h->size;
    return (arena_header_t *)((char *)h + total);
}

static arena_header_t *prev_header(arena_header_t *h) {
    if (h->prev_size == 0) {
        return NULL;
    }
    size_t prev_total = header_size() + h->prev_size;
    return (arena_header_t *)((char *)h - prev_total);
}

static void merge_with_next(arena_header_t *h) {
    arena_header_t *n = next_header(h);
    if ((char *)n >= (char *)g_arena + g_arena->size) {
        return;
    }
    if (n->magic != ARENA_MAGIC) {
        return;
    }
    if (!n->free) {
        return;
    }
    remove_free_list(n);
    h->size += header_size() + n->size;
    if (g_arena->last == n) {
        g_arena->last = h;
    }
}

static void merge_with_prev(arena_header_t *h) {
    arena_header_t *p = prev_header(h);
    if (!p) {
        return;
    }
    if (!p->free) {
        return;
    }
    remove_free_list(p);
    p->size += header_size() + h->size;
    h->prev_size = 0;
    if (g_arena->last == h) {
        g_arena->last = p;
    }
}

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(arena_state_t) + header_size()) {
        g_arena = NULL;
        return;
    }
    arena_state_t *state = (arena_state_t *)buf;
    state->magic = ARENA_MAGIC;
    state->size = size;
    state->first = NULL;
    state->last = NULL;
    g_arena = state;

    char *start = (char *)buf + sizeof(arena_state_t);
    size_t remaining = size - sizeof(arena_state_t);
    if (remaining < header_size()) {
        return;
    }
    arena_header_t *h = (arena_header_t *)start;
    h->magic = ARENA_MAGIC;
    h->size = remaining - header_size();
    h->prev_size = 0;
    h->free = 1;
    h->next = NULL;
    h->prev = NULL;
    state->first = h;
    state->last = h;
}

void *arena_alloc(size_t n) {
    if (!g_arena || n == 0) {
        return NULL;
    }
    size_t align = _Alignof(max_align_t);
    size_t needed = align_up(n, align);
    size_t total = header_size() + needed;

    arena_header_t *h = g_arena->first;
    while (h) {
        if (h->size >= needed) {
            if (h->size > needed + header_size()) {
                size_t split_at = needed;
                arena_header_t *new_h = (arena_header_t *)((char *)h + header_size() + split_at);
                new_h->magic = ARENA_MAGIC;
                new_h->size = h->size - split_at - header_size();
                new_h->prev_size = split_at;
                new_h->free = 1;
                new_h->next = h->next;
                new_h->prev = h->prev;
                if (h->next) {
                    h->next->prev = new_h;
                }
                if (h->prev) {
                    h->prev->next = new_h;
                } else {
                    g_arena->first = new_h;
                }
                h->next = new_h;
                h->size = split_at;
                if (g_arena->last == h) {
                    g_arena->last = new_h;
                }
            }
            remove_free_list(h);
            return payload_from_header(h);
        }
        h = h->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p || !g_arena) {
        return;
    }
    arena_header_t *h = header_from_payload(p);
    if (h->magic != ARENA_MAGIC) {
        return;
    }
    insert_free_list(h);
    merge_with_next(h);
    merge_with_prev(h);
}

void *arena_realloc(void *p, size_t n) {
    if (!p) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!g_arena) {
        return NULL;
    }
    arena_header_t *h = header_from_payload(p);
    if (h->magic != ARENA_MAGIC) {
        return NULL;
    }
    size_t align = _Alignof(max_align_t);
    size_t needed = align_up(n, align);
    if (h->size >= needed) {
        if (h->size > needed + header_size()) {
            size_t split_at = needed;
            arena_header_t *new_h = (arena_header_t *)((char *)h + header_size() + split_at);
            new_h->magic = ARENA_MAGIC;