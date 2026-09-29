#include <stddef.h>
#include <stdint.h>

typedef struct arena {
    void *buf;
    size_t size;
    void *free;
    size_t next_alloc;
    size_t next_free;
} arena_t;

#define ARENA_ALIGN 16

void arena_init(void *buf, size_t size) {
    arena_t *a = (arena_t *)buf;
    a->buf = buf;
    a->size = size;
    a->free = a->buf;
    a->next_alloc = 0;
    a->next_free = 0;
}

void *arena_alloc(size_t n) {
    arena_t *a = (arena_t *)((uint8_t *)a->buf + a->next_alloc);
    size_t s = n + ARENA_ALIGN;
    if (a->free == NULL || (size_t)a->free + s > a->size)
        return NULL;
    void *p = (void *)((uint8_t *)a->free + ARENA_ALIGN);
    if ((uint8_t *)p % ARENA_ALIGN)
        p = (void *)((uint8_t *)p + ARENA_ALIGN);
    a->next_alloc = (size_t)p + s;
    a->free = (void *)((uint8_t *)p + n);
    return p;
}

void arena_free(void *p) {
    arena_t *a = (arena_t *)((uint8_t *)a->buf + a->next_alloc);
    if (p == NULL)
        return;
    size_t s = (size_t)a->free - (size_t)p;
    if (s > n)
        a->free = p;
    else
        a->free = (void *)((uint8_t *)a->free + n);
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL)
        return arena_alloc(n);
    arena_t *a = (arena_t *)((uint8_t *)a->buf + a->next_alloc);
    size_t s = (size_t)a->free - (size_t)p;
    if (s >= n) {
        if (s > n)
            a->free = (void *)((uint8_t *)p + n);
        return p;
    }
    void *np = arena_alloc(n);
    if (np == NULL)
        return NULL;
    memcpy(np, p, s);
    return np;
}