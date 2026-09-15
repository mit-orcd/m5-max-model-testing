#include <stdint.h>
#include <stddef.h>

#define ALIGN sizeof(int)

typedef struct {
    void *buf;
    size_t size;
    size_t free;
} Arena;

static Arena _arena;

void arena_init(void *buf, size_t size) {
    _arena.buf = buf;
    _arena.size = size;
    _arena.free = size;
}

static size_t align_up(size_t n) {
    return (n + ALIGN - 1) / ALIGN * ALIGN;
}

static void *find_free(size_t n, size_t *out_pos, size_t *out_size) {
    char *b = (char *)_arena.buf;
    size_t pos = 0;
    size_t remaining = _arena.size;

    while (remaining > 0) {
        size_t sz = 0;
        if (pos < _arena.size) {
            sz = *(size_t *)(b + pos);
        }
        if (sz == 0) {
            size_t free_sz = remaining;
            if (free_sz >= n) {
                *out_pos = pos;
                *out_size = free_sz;
                return (void *)b;
            }
        } else {
            pos += ALIGN + sz;
            remaining = _arena.size - pos;
        }
    }
    return NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t an = align_up(n);
    size_t pos, size;
    void *b = find_free(an, &pos, &size);
    if (!b) return NULL;
    *(size_t *)((char *)b + pos) = an;
    _arena.free -= size;
    return (void *)((char *)b + pos + ALIGN);
}

void arena_free(void *p) {
    if (!p) return;
    char *b = (char *)_arena.buf;
    size_t pos = (size_t)p - ALIGN;
    *(size_t *)(b + pos) = 0;
    _arena.free += align_up(*(size_t *)(b + pos)) + ALIGN;

    size_t total = _arena.size;
    size_t next_pos = pos + ALIGN + *(size_t *)(b + pos);
    if (next_pos < total && *(size_t *)(b + next_pos) == 0) {
        size_t next_sz = align_up(*(size_t *)(b + next_pos)) + ALIGN;
        *(size_t *)(b + pos) += next_sz;
        _arena.free += next_sz;
        next_pos += next_sz;
    }

    if (pos >= ALIGN) {
        size_t prev_pos = pos - ALIGN - *(size_t *)(b + (pos - ALIGN));
        if (prev_pos >= 0 && *(size_t *)(b + prev_pos) == 0) {
            size_t prev_sz = align_up(*(size_t *)(b + prev_pos)) + ALIGN;
            *(size_t *)(b + prev_pos) += ALIGN + *(size_t *)(b + pos);
            _arena.free += ALIGN;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }

    char *b = (char *)_arena.buf;
    size_t old_pos = (size_t)p - ALIGN;
    size_t old_alloc = *(size_t *)(b + old_pos);
    size_t old_usable = old_alloc;
    size_t an = align_up(n);

    if (an <= old_usable) {
        if (an < old_usable) {
            *(size_t *)(b + old_pos) = an;
            _arena.free += old_usable - an;
        }
        return p;
    }

    size_t next_pos = old_pos + ALIGN + old_alloc;
    size_t total = _arena.size;
    if (next_pos < total && *(size_t *)(b + next_pos) == 0) {
        size_t next_free = total - next_pos;
        if (next_free >= an) {
            memmove(p + old_usable, (char *)p + an, old_usable - an);
            *(size_t *)(b + old_pos) = an;
            _arena.free -= an - old_usable;
            return p;
        }
    }

    void *np = arena_alloc(n);
    if (!np) return NULL;
    size_t copy_sz = old_usable < an ? old_usable : an;
    char *src = (char *)p;
    char *dst = (char *)np;
    for (size_t i = 0; i < copy_sz; i++) {
        dst[i] = src[i];
    }
    arena_free(p);
    return np;
}