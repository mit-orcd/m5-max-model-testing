#include <stddef.h>
#include <stdint.h>

typedef struct block {
    size_t size;      /* total block size including header */
    uint8_t free;     /* 1 if free, 0 if in use */
    uint8_t align_pad;/* padding to align header to max_align_t */
    uint8_t pad[sizeof(size_t) - 1];
} block_t;

#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((a) - 1))
#define ALIGN_MASK(a) ((a) - 1)

static block_t *arena_base;
static size_t arena_size;
static size_t arena_align;

static void *align_ptr(void *p) {
    return (void *)(((uintptr_t)p + ALIGN_MASK(arena_align) - 1) & ~(uintptr_t)ALIGN_MASK(arena_align));
}

static block_t *block_at(void *p) {
    return (block_t *)((uintptr_t)p - offsetof(block_t, size));
}

static void *block_data(block_t *b) {
    return (void *)((uintptr_t)b + arena_align);
}

static void coalesce(block_t *b) {
    block_t *next = (block_t *)((uintptr_t)block_data(b) + b->size - arena_align);
    if ((uintptr_t)next < (uintptr_t)arena_base + arena_size) {
        if (next->free) {
            b->size += next->size;
        }
    }
    if ((uintptr_t)b > (uintptr_t)arena_base) {
        block_t *prev = (block_t *)((uintptr_t)b - prev_size(b));
        if (prev->free) {
            prev->size += b->size;
            b = prev;
        }
    }
}

static size_t prev_size(block_t *b) {
    return ((block_t *)b)[-1].size;
}

void arena_init(void *buf, size_t size) {
    arena_base = (block_t *)align_ptr(buf);
    arena_size = ALIGN_UP(size, arena_align);
    arena_align = sizeof(block_t);
    if (arena_align < sizeof(max_align_t))
        arena_align = sizeof(max_align_t);
    block_t *b = arena_base;
    b->size = arena_size;
    b->free = 1;
    b->align_pad = 0;
    b->pad[0] = 0;
}

void *arena_alloc(size_t n) {
    if (n == 0)
        return NULL;
    size_t total = ALIGN_UP(n, arena_align) + arena_align;
    block_t *b = arena_base;
    while ((uintptr_t)b < (uintptr_t)arena_base + arena_size) {
        if (b->free && b->size >= total) {
            if (b->size >= total + arena_align) {
                b->size -= arena_align;
                b->free = 0;
                return block_data(b);
            } else {
                b->free = 0;
                return block_data(b);
            }
        }
        b = (block_t *)((uintptr_t)block_data(b) + b->size - arena_align);
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL)
        return;
    block_t *b = block_at(p);
    b->free = 1;
    coalesce(b);
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL)
        return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    block_t *b = block_at(p);
    size_t old_size = b->size - arena_align;
    size_t total = ALIGN_UP(n, arena_align) + arena_align;
    if (old_size >= total) {
        /* shrink in place */
        if (b->size - total >= arena_align) {
            b->size -= arena_align;
        }
        return p;
    }
    /* try to grow into next free neighbour */
    block_t *next = (block_t *)((uintptr_t)block_data(b) + b->size - arena_align);
    if ((uintptr_t)next < (uintptr_t)arena_base + arena_size && next->free &&
        next->size >= total - b->size) {
        b->size += next->size;
        return p;
    }
    /* allocate new and copy */
    void *np = arena_alloc(n);
    if (np == NULL)
        return NULL;
    if (old_size > n)
        old_size = n;
    __builtin_memcpy(np, p, old_size);
    arena_free(p);
    return np;
}