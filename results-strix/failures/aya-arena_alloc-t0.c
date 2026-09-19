#include <stddef.h>
#include <stdint.h>

typedef struct arena {
    void *buf;
    size_t size;
    void *alloc;
    size_t used;
    size_t max;
} arena_t;

void arena_init(void *buf, size_t size) {
    arena_t *a = (arena_t *)buf;
    a->buf = buf;
    a->size = size;
    a->alloc = a->buf;
    a->used = 0;
    a->max = 0;
}

void *arena_alloc(size_t n) {
    arena_t *a = (arena_t *)((char *)a->buf - sizeof(arena_t));
    size_t s = n + _Alignof(max_align_t) - 1;
    s &= ~_Alignof(max_align_t) - 1;
    if (a->used + s > a->size)
        return NULL;
    void *p = a->alloc;
    a->alloc = (char *)a->alloc + s;
    a->used += s;
    if (a->max < s)
        a->max = s;
    return p;
}

void arena_free(void *p) {
    arena_t *a = (arena_t *)((char *)p - sizeof(arena_t));
    size_t s = a->alloc - (char *)p;
    if (s > a->max)
        a->max = s;
    a->alloc = p;
    a->used -= s;
}

void *arena_realloc(void *p, size_t n) {
    arena_t *a = (arena_t *)((char *)p - sizeof(arena_t));
    size_t s = n + _Alignof(max_align_t) - 1;
    s &= ~_Alignof(max_align_t) - 1;
    if (a->used + s > a->size)
        return NULL;
    if (p == a->alloc && a->alloc + s >= (char *)p + n) {
        a->alloc += s;
        a->used += s;
    }
    return p;
}