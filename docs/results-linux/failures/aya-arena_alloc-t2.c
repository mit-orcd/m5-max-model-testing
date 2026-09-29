#include <stddef.h>
#include <stdint.h>

typedef struct arena {
    void *buf;
    size_t size;
    void *alloc_next;
    void *free_next;
} arena_t;

// Initialize an arena.
void arena_init(void *buf, size_t size) {
    arena_t *a = (arena_t *)buf;
    a->buf = buf;
    a->size = size;
    a->alloc_next = a->buf;
    a->free_next = NULL;
}

// Allocate 'n' bytes from the arena.
void *arena_alloc(size_t n) {
    arena_t *a = (arena_t *)((char *)a->alloc_next - sizeof(arena_t));
    if (n == 0)
        return NULL;
    if (a->alloc_next + n > a->buf + a->size)
        return NULL;
    a->alloc_next += _Alignof(max_align_t);
    void *p = a->alloc_next;
    a->alloc_next += n;
    return p;
}

// Free a block of memory allocated from this arena.
void arena_free(void *p) {
    arena_t *a = (arena_t *)((char *)p - sizeof(arena_t));
    if (p) {
        a->alloc_next = p;
        if (a->free_next) {
            void *next = a->free_next;
            a->free_next = p;
            if (next == a->alloc_next) {
                a->alloc_next = a->free_next;
                a->free_next = NULL;
            }
        } else {
            a->free_next = p;
        }
    }
}

// Reallocate a block of memory allocated from this arena.
void *arena_realloc(void *p, size_t n) {
    if (p == NULL)
        return arena_alloc(n);
    arena_t *a = (arena_t *)((char *)p - sizeof(arena_t));
    if (a->alloc_next != p)
        return NULL;  // not allocated from this arena
    size_t old_n = a->alloc_next - p;
    if (n <= old_n)
        return p;
    if (a->alloc_next + n > a->buf + a->size)
        return NULL;
    void *new_p = arena_alloc(n);
    if (new_p)
        memcpy(new_p, p, old_n);
    arena_free(p);
    return new_p;
}