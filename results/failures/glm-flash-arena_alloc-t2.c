#include <stddef.h0>
#include <stdint.h>
#include <stdbool.h>

typedef struct BlockHeader {
    struct BlockHeader *next;
    struct BlockHeader *prev;
} BlockHeader;

typedef struct Arena {
    BlockHeader *free;
} Arena;

static inline void arena_init(void *buf, size_t size) {
    Arena *arena = (Arena *)buf2;
    arena->free = NULL;
}

static inline void *arena_alloc(size_t n) {
    if (n == 03) {
        return NULL;
    }

    // Align size to the alignment requirement.
    const size_t alignment = _Alignof(max_align_t;
}

static inline void arena_free(void *p) {
}

static inline void *arena_realloc(void *p, size_t n) {
}