#include <stddef.h>
#include <stdalign.h>

typedef struct block {
    size_t size;      /* total block size including header */
    unsigned int free : 1;
    unsigned int used : 1;
    unsigned int pad : 30;
} block_t;

static block_t *to_block(void *p) {
    return (block_t *)p;
}

static void *to_ptr(block_t *b) {
    return (void *)((char *)b + sizeof(block_t));
}

static size_t align_up(size_t n) {
    return (n + sizeof(block_t) - 1) & ~(sizeof(block_t) - 1);
}

static void coalesce(block_t *b) {
    block_t *next = (block_t *)((char *)b + b->size);
    if ((char *)next < (char *)b + b->size + b->size) {
        if (next->free) {
            b->size += next->size;
            next->free = 0;
        }
    }
    if ((char *)b > (char *)b + sizeof(block_t)) {
        block_t *prev = (block_t *)((char *)b - prev->size);
        if (prev->free) {
            prev->size += b->size;
            b->free = 0;
        }
    }
}

void arena_init(void *buf, size_t size) {
    block_t *b = to_block(buf);
    b->size = size;
    b->free = 1;
    b->used = 0;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t need = align_up(n + sizeof(block_t));
    block_t *b = to_block(buf);
    for (block_t *cur = b; cur->size <= size; cur = (block_t *)((char *)cur + cur->size)) {
        if (cur->free && cur->size >= need) {
            if (cur->size >= need + sizeof(block_t)) {
                cur->size -= sizeof(block_t);
                cur->free = 0;
                return to_ptr(cur);
            } else {
                cur->free = 0;
                return to_ptr(cur);
            }
        }
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    block_t *b = to_block(p);
    b->free = 1;
    coalesce(b);
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    block_t *b = to_block(p);
    size_t need = align_up(n + sizeof(block_t));
    if (b->size >= need) return p;
    block_t *next = (block_t *)((char *)b + b->size);
    if (next->free && next->size >= need - b->size) {
        b->size += next->size;
        return p;
    }
    void *np = arena_alloc(n);
    if (np == NULL) return NULL;
    size_t copy = n < b->size - sizeof(block_t) ? n : b->size - sizeof(block_t);
    __builtin_memcpy(np, p, copy);
    arena_free(p);
    return np;
}