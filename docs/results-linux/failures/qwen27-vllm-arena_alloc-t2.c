#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((size_t)(a) - 1))

typedef struct {
    struct {
        size_t size;
        int free;
        struct header *next;
        struct header *prev;
    } header;
    uint8_t data[];
} header_t;

typedef struct {
    header_t *first;
    header_t *last;
    size_t total;
} arena_t;

static arena_t *arena_state = NULL;

static size_t align_size(size_t n) {
    return ALIGN_UP(n, _Alignof(max_align_t));
}

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(arena_t) + sizeof(header_t) + _Alignof(max_align_t)) {
        arena_state = NULL;
        return;
    }

    /* Align the buffer start for arena_t */
    uintptr_t raw = (uintptr_t)buf;
    uintptr_t aligned = ALIGN_UP(raw, _Alignof(max_align_t));
    size_t pad = (size_t)(aligned - raw);

    if (pad + sizeof(arena_t) + sizeof(header_t) + _Alignof(max_align_t) > size) {
        arena_state = NULL;
        return;
    }

    arena_t *a = (arena_t *)aligned;
    a->total = size - pad;
    a->first = NULL;
    a->last = NULL;

    /* The heap area starts right after the arena_t structure */
    uintptr_t heap_start = (uintptr_t)(a + 1);
    uintptr_t heap_aligned = ALIGN_UP(heap_start, _Alignof(max_align_t));
    size_t heap_pad = (size_t)(heap_aligned - heap_start);

    if (heap_pad + sizeof(header_t) + _Alignof(max_align_t) > a->total) {
        arena_state = NULL;
        return;
    }

    /* Create a single free block covering the entire heap */
    header_t *h = (header_t *)heap_aligned;
    size_t data_size = a->total - heap_pad - sizeof(header_t);
    h->header.size = data_size;
    h->header.free = 1;
    h->header.next = NULL;
    h->header.prev = NULL;

    a->first = h;
    a->last = h;

    arena_state = a;
}

static header_t *get_header(void *p) {
    if (!p) return NULL;
    header_t *h = (header_t *)p - 1;
    return h;
}

static void *get_data(header_t *h) {
    return (void *)(h + 1);
}

static size_t header_overhead(void) {
    return sizeof(header_t);
}

void *arena_alloc(size_t n) {
    if (!arena_state || n == 0) return NULL;

    size_t needed = align_size(n) + header_overhead();
    if (needed > arena_state->total) return NULL;

    header_t *h = arena_state->first;
    while (h) {
        if (h->header.free && h->header.size >= align_size(n)) {
            size_t data_size = h->header.size;
            size_t alloc_data = align_size(n);

            /* Check if we can split the block */
            if (data_size - alloc_data >= header_overhead() + _Alignof(max_align_t)) {
                /* Split: create a new free block for the remainder */
                header_t *new_h = (header_t *)((uint8_t *)get_data(h) + alloc_data);
                size_t new_size = data_size - alloc_data - header_overhead();

                new_h->header.size = new_size;
                new_h->header.free = 1;
                new_h->header.next = h->header.next;
                new_h->header.prev = h;

                h->header.size = alloc_data;
                h->header.free = 0;
                h->header.next = new_h;

                if (new_h->header.next) {
                    new_h->header.next->header.prev = new_h;
                } else {
                    arena_state->last = new_h;
                }
            } else {
                /* Cannot split, use the whole block */
                h->header.free = 0;
            }

            return get_data(h);
        }
        h = h->header.next;
    }

    return NULL;
}

static void coalesce(header_t *h) {
    /* Merge with next if free */
    while (h->header.next && h->header.next->header.free) {
        header_t *next = h->header.next;
        h->header.size += header_overhead() + next->header.size;
        h->header.next = next->header.next;
        if (h->header.next) {
            h->header.next->header.prev = h;
        } else {
            arena_state->last = h;
        }
    }

    /* Merge with prev if free */
    while (h->header.prev && h->header.prev->header.free) {
        header_t *prev = h->header.prev;
        prev->header.size += header_overhead() + h->header.size;
        prev->header.next = h->header.next;
        if (prev->header.next) {
            prev->header.next->header.prev = prev;
        } else {
            arena_state->last = prev;
        }
        h = prev;
    }
}

void arena_free(void *p) {
    if (!p) return;
    if (!arena_state) return;

    header_t *h = get_header(p);
    h->header.free = 1;
    coalesce(h);
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        if (p) arena_free(p);
        return NULL;
    }

    if (!p) return arena_alloc(n);

    header_t *h = get_header(p);
    size_t old_size = h->header.size;
    size_t new_size = align_size(n);

    /* If the new size fits in the current block, just update size */
    if (new_size <= old_size) {
        h->header.size = new_size;
        return p;
    }

    /* Check if we can grow into the next block if it's free and large enough */
    if (h->header.next && h->header.next->header.free) {
        header_t *next = h->header.next;
        size_t combined = old_size + header_overhead() + next->header.size;
        if (combined >= new_size) {
            /* Grow into next */
            size_t remainder = combined - new_size;
            if (remainder >= header_overhead() + _Alignof(max_align_t)) {
                /* Split the remainder */
                header_t *new_h = (header_t *)((uint8_t *)get_data(h) + new_size);
                new_h->header.size = remainder - header_overhead();
                new_h->header.free = 1;
                new_h->header.next = next->header.next;
                new_h->header.prev = h;

                h->header.size = new_size;
                h->header.next = new_h;

                if (new_h->header.next) {
                    new_h->header.next->header.prev = new_h;
                } else {
                    arena_state->last = new_h;
                }
            } else {
                /* Use the entire combined space */
                h->header.size = new_size;
                h->header.next = next->header.next;
                if (h->header.next) {
                    h->header.next->header.prev = h;
                } else {
                    arena_state->last = h;
                }
            }
            return p;
        }
    }

    /* Need to move the block */
    void *new_p = arena_alloc(n);
    if (!new_p) return NULL;

    size_t copy_size = old_size < new_size ? old_size : new_size;
    memcpy(new_p, p, copy_size);
    arena_free(p);
    return new_p;
}