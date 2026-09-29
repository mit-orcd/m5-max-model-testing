#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct Arena {
    void *buf;
    size_t size;
    void *free;
    size_t used;
    size_t max;
} Arena;

void arena_init(void *buf, size_t size) {
    Arena *a = (Arena *)buf;
    a->buf = buf;
    a->size = size;
    a->free = a->buf;
    a->used = 0;
    a->max = size;
}

void *arena_alloc(size_t n) {
    Arena *a = (Arena *)((char *)a->buf + sizeof(Arena));
    if (a->free == NULL || n > a->max - a->used)
        return NULL;
    void *p = a->free;
    size_t s = n + sizeof(size_t); // align(s)
    if (s % _Alignof(max_align_t) != 0)
        s = _Alignof(max_align_t) * ((s + _Alignof(max_align_t) - 1) / _Alignof(max_align_t));
    a->free = (char *)p + s;
    *(size_t *)p = s;
    a->used += s;
    return (char *)p + sizeof(size_t);
}

void arena_free(void *p) {
    Arena *a = (Arena *)((char *)a->buf - sizeof(Arena));
    if (p == NULL)
        return;
    size_t s = *(size_t *)p - sizeof(size_t);
    *(size_t *)p = 0;
    a->free = p;
    a->used -= s;
}

void *arena_realloc(void *p, size_t n) {
    Arena *a = (Arena *)((char *)a->buf + sizeof(Arena));
    if (p == NULL)
        return arena_alloc(n);
    size_t s = *(size_t *)p - sizeof(size_t);
    if (n <= s)
        return p;
    void *np = arena_alloc(n);
    if (np == NULL)
        return NULL;
    memcpy(np, p, s < n ? s : n);
    arena_free(p);
    return np;
}