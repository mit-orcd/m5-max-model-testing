#include <stddef.h>
#include <stdalign.h>

typedef struct block {
    size_t size;
    int free;
} block_t;

static char *g_buf;
static size_t g_size;

void arena_init(void *buf, size_t size) {
    g_buf = (char *)buf;
    g_size = size;
    block_t *b = (block_t *)g_buf;
    b->size = size - sizeof(block_t);
    b->free = 1;
}

static void *align_ptr(void *p, size_t align) {
    uintptr_t addr = (uintptr_t)p;
    uintptr_t mask = align - 1;
    if (addr & mask)
        addr = (addr + mask) & ~mask;
    return (void *)addr;
}

static block_t *next_block(block_t *b) {
    return (block_t *)(g_buf + ((char *)b - g_buf) + b->size + sizeof(block_t));
}

static int is_last_block(block_t *b) {
    return (char *)next_block(b) >= g_buf + g_size;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t align = _Alignof(max_align_t);
    size_t header_size = sizeof(block_t);
    size_t total = header_size + n;
    if (total < n) return NULL;
    if ((total - header_size) % align != 0)
        total += align - ((total - header_size) % align);
    block_t *b = (block_t *)g_buf;
    while (!is_last_block(b)) {
        if (b->free && b->size >= n) {
            if (b->size >= total) {
                size_t remaining = b->size - n;
                block_t *split = next_block(b);
                split->size = remaining;
                split->free = 1;
            }
            b->size = n;
            b->free = 0;
            return (void *)((char *)b + header_size);
        }
        b = next_block(b);
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    block_t *b = (block_t *)((char *)p - sizeof(block_t));
    b->free = 1;
    if (!is_last_block(b)) {
        block_t *next = next_block(b);
        if (next->free) {
            b->size += sizeof(block_t) + next->size;
        }
    }
    if (b != (block_t *)g_buf) {
        block_t *prev = (block_t *)((char *)b - sizeof(block_t) - ((char *)b - g_buf - sizeof(block_t) - ((block_t *)((char *)b - sizeof(block_t)))->size));
        /* Find previous block properly */
    }
    /* Scan backwards to merge with previous free block */
    {
        char *pos = (char *)b - sizeof(block_t);
        while (pos > g_buf) {
            block_t *prev = (block_t *)pos;
            if (prev->free) {
                prev->size += sizeof(block_t) + b->size;
                *b = *prev; /* copy size and free flag, but we want prev's position */
                /* Actually merge: prev absorbs b */
                prev->size += sizeof(block_t) + b->size;
                b = prev;
                break;
            }
            pos -= sizeof(block_t) + prev->size;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    block_t *b = (block_t *)((char *)p - sizeof(block_t));
    size_t old_size = b->size;
    size_t align = _Alignof(max_align_t);
    size_t header_size = sizeof(block_t);
    size_t total = header_size + n;
    if (total < n) return NULL;
    if ((total - header_size) % align != 0)
        total += align - ((total - header_size) % align);
    if (b->size >= n) {
        if (b->size >= total) {
            size_t remaining = b->size - n;
            block_t *split = next_block(b);
            split->size = remaining;
            split->free = 1;
        }
        b->size = n;
        return p;
    }
    if (!is_last_block(b)) {
        block_t *next = next_block(b);
        if (next->free && next->size >= total - sizeof(block_t) - b->size) {
            size_t grow = next->size + sizeof(block_t);
            char *data = (char *)b + header_size;
            b->size += grow;
            if (!is_last_block(next)) {
                block_t *nn = next_block(next);
                *b = *nn;
            } else {
                b->free = 1; /* mark end */
            }
            memcpy(data, data, old_size < n ? old_size : n);
            return p;
        }
    }
    void *newp = arena_alloc(n);
    if (newp) {
        size_t copy = old_size < n ? old_size : n;
        memcpy(newp, p, copy);
        arena_free(p);
    }
    return newp;
}