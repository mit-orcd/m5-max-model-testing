#include <stddef.h>
#include <stdint.h>

typedef struct arena {
    void *buf;
    size_t size;
    void *alloc;
    size_t used;
    size_t free;
} arena_t;

void arena_init(void *buf, size_t size) {
    arena_t *a = (arena_t *)buf;
    a->buf = buf;
    a->size = size;
    a->alloc = a->buf;
    a->used = 0;
    a->free = size;
}

void *arena_alloc(size_t n) {
    arena_t *a = (arena_t *)((char *)a->buf - sizeof(arena_t));
    if (n == 0) return NULL;
    if (a->free < n + _Alignof(max_align_t)) return NULL;
    void *p = a->alloc;
    a->alloc = (char *)a->alloc + n;
    a->used += n;
    a->free -= n;
    return p;
}

void arena_free(void *p) {
    arena_t *a = (arena_t *)((char *)p - sizeof(arena_t));
    if (p == NULL) return;
    size_t n = (char *)a->alloc - (char *)p;
    a->alloc = p;
    a->used -= n;
    a->free += n;
    if (n < _Alignof(max_align_t)) return;
    size_t i = (char *)a->alloc - (char *)a->buf;
    if (i < a->used && a->free > _Alignof(max_align_t)) {
        a->alloc = (char *)a->buf + i;
        a->used = 0;
        a->free = a->size;
    }
}

void *arena_realloc(void *p, size_t n) {
    arena_t *a = (arena_t *)((char *)p - sizeof(arena_t));
    if (p == NULL) return arena_alloc(n);
    size_t old = (char *)a->alloc - (char *)p;
    if (n <= old) return p;
    if (a->free < n + _Alignof(max_align_t)) return NULL;
    void *q = a->alloc;
    a->alloc = (char *)a->alloc + n;
    a->used += n - old;
    a->free -= n - old;
    return p;
}