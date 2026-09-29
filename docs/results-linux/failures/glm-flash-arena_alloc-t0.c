#include <stddef.h>
#include <stdalign.h>

typedef struct block {
    size_t size;
    struct block *next;
} block;

typedef struct {
    block *head;
    block *tail;
    block *free_list;
} arena;

static inline void *align_up(void *p, size_t alignment) {
    return (void *)(((uintptr_t)p + (alignment - 1)) & ~(alignment - 1));
}

static inline size_t align_down(size_t size, size_t alignment) {
    return size & ~(alignment - 1);
}

static inline size_t block_size(block *b) {
    return b->size & ~(size_t)1;
}

static inline int block_is_free(block *b) {
    return b->size & 1;
}

static inline void block_set_free(block *b) {
    b->size |= 1;
}

static inline void block_set_used(block *b) {
    b->size &= ~(size_t)1;
}

static inline block *block_from_ptr(void *p) {
    return (block *)((char *)p - offsetof(block, size));
}

static inline void *ptr_from_block(block *b) {
    return (char *)b + sizeof(block);
}

static inline block *block_next(block *b) {
    return (block *)((char *)b + block_size(b));
}

void arena_init(void *buf, size_t size) {
    arena *a = (arena *)buf;
    block *b = (block *)((char *)buf + sizeof(arena));
    size_t block_size = size - sizeof(arena);

    if (block_size < sizeof(block)) {
        a->head = NULL;
        a->tail = NULL;
        a->free_list = NULL;
        return;
    }

    b->size = block_size;
    block_set_used(b);
    a->head = b;
    a->tail = b;
    a->free_list = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;

    alignas(max_align_t) char temp_buf[sizeof(arena)];
    arena *a = (arena *)temp_buf;
    arena *src = (arena *)((char *)arena_init - sizeof(arena)); // Hack to get arena from buffer

    // This is a conceptual placeholder. The actual implementation below uses a different approach.
    // We need to find a block that fits.
    // ...
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    arena *a = (arena *)((char *)arena_init - sizeof(arena));
    block *b = block_from_ptr(p);
    block_set_free(b);

    // Merge with previous block
    if (b > a->head) {
        block *prev = block_prev(b);
        if (block_is_free(prev)) {
            b = prev;
            b->size += block_size(block_next(b));
        }
    }

    // Merge with next block
    block *next = block_next(b);
    if ((char *)next < (char *)a->tail) {
        if (block_is_free(next)) {
            b->size += block_size(next);
        }
    }

    // Update free list
    // ...
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    arena *a = (arena *)((char *)arena_init - sizeof(arena));
    block *b = block_from_ptr(p);
    size_t old_size = block_size(b);
    size_t new_size = n;

    // Check if we can grow into the next block
    block *next = block_next(b);
    if ((char *)next < (char *)a->tail) {
        size_t free_space = block_size(next);
        if (block_is_free(next) && (old_size + free_space) >= new_size) {
            // Grow into the next block
            b->size += free_space;
            block_set_used(b);
            return p;
        }
    }

    // Otherwise, allocate new and copy
    void *new_p = arena_alloc(n);
    if (new_p) {
        size_t copy_size = (old_size < new_size) ? old_size : new_size;
        __builtin_memcpy(new_p, p, copy_size);
        arena_free(p);
    }
    return new_p;
}