#include <stddef.h>
#include <stdint.h>

// fixed-buffer allocator

typedef struct arena {
    void *next;  // next free block
    void *end;   // end of arena
} arena_t;

// initialize arena
void arena_init(void *buf, size_t size) {
    arena_t *a = (arena_t *)buf;
    a->next = buf;
    a->end = buf + size;
}

// allocate memory block
void *arena_alloc(size_t n) {
    arena_t *a = (arena_t *)((char *)a->next - _Alignof(max_align_t));
    size_t s = _Alignof(max_align_t) + n;  // aligned size
    if (a->next + s > a->end) return NULL;  // request too large
    a->next += s;
    return a->next - s;  // aligned pointer
}

// free memory block
void arena_free(void *p) {
    arena_t *a = (arena_t *)((char *)p - _Alignof(max_align_t));
    if (a->next == a->end) return;  // last block, cannot merge
    if (a->next == a->end - _Alignof(max_align_t)) {
        a->next = a->end;  // merge with end
    } else {
        void *q = a->next;
        size_t s = _Alignof(max_align_t);
        while (_Alignof(max_align_t) <= (uintptr_t)q % _Alignof(max_align_t)) {
            s += _Alignof(max_align_t);
            q += _Alignof(max_align_t);
        }
        a->next = q;
        if (q == a->end - s) a->next = a->end;  // merge with end or previous
    }
}

// resize memory block
void *arena_realloc(void *p, size_t n) {
    arena_t *a = (arena_t *)((char *)p - _Alignof(max_align_t));
    size_t s = _Alignof(max_align_t) + n;  // aligned size
    if (a->next + s > a->end) return NULL;  // request too large
    if (a->next == a->end - _Alignof(max_align_t)) {
        // grow into neighbour
        a->next = a->end;
        return p;
    }
    void *q = a->next;
    while (_Alignof(max_align_t) <= (uintptr_t)q % _Alignof(max_align_t)) {
        q += _Alignof(max_align_t);
    }
    if (q == a->end - s) {
        // grow into neighbour
        a->next = a->end;
        return p;
    }
    // copy and adjust pointers
    char *d = (char *)a->next;
    char *e = (char *)a->next + s;
    while (d < e) {
        char c = *d;
        *d++ = *q;
        *q++ = c;
    }
    a->next += s;
    return p;
}