#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ARENA_ALIGNMENT _Alignof(max_align_t)

typedef struct {
    size_t size;
    char data[];
} arena_header_t;

static arena_header_t *arena = NULL;

void arena_init(void *buf, size_t size) {
    if (buf == NULL || size < sizeof(arena_header_t)) {
        arena = NULL;
        return;
    }
    arena = (arena_header_t *)buf;
    arena->size = size - sizeof(arena_header_t);
}

void *arena_alloc(size_t n) {
    if (arena == NULL || n == 0 || n > arena->size) {
        return NULL;
    }

    void *ptr = arena->data;
    size_t remaining = arena->size;

    while (remaining >= sizeof(arena_header_t)) {
        arena_header_t *header = (arena_header_t *)ptr;
        size_t block_size = header->size;

        if (block_size >= n) {
            if (block_size > n + sizeof(arena_header_t)) {
                header->size = block_size - n - sizeof(arena_header_t);
                void *next_ptr = (char *)ptr + n + sizeof(arena_header_t);
                arena_header_t *next_header = (arena_header_t *)next_ptr;
                next_header->size = header->size;
            }
            ptr = (char *)ptr + sizeof(arena_header_t);
            return ptr;
        }

        ptr = (char *)ptr + block_size + sizeof(arena_header_t);
        remaining -= block_size + sizeof(arena_header_t);
    }

    return NULL;
}

void arena_free(void *p) {
    if (p == NULL || arena == NULL) {
        return;
    }
    p = (char *)p - sizeof(arena_header_t);
    arena_header_t *header = (arena_header_t *)p;
    header->size = (size_t)(arena->data + arena->size - (char *)p) - sizeof(arena_header_t);
}

void *arena_realloc(void *p, size_t n) {
    if (arena == NULL) {
        return NULL;
    }
    if (p == NULL) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    void *ptr = (char *)p - sizeof(arena_header_t);
    arena_header_t *header = (arena_header_t *)ptr;

    if (n <= header->size) {
        return p;
    }

    size_t old_size = header->size;
    void *next_ptr = (char *)p + old_size;
    arena_header_t *next_header = (arena_header_t *)next_ptr;

    if ((char *)next_ptr + sizeof(arena_header_t) > arena->data + arena->size) {
        void *new_buf = arena_alloc(n);
        if (new_buf == NULL) {
            return NULL;
        }
        memcpy(new_buf, p, old_size);
        return new_buf;
    }

    if (next_header->size == 0) {
        void *new_buf = arena_alloc(n);
        if (new_buf == NULL) {
            return NULL;
        }
        memcpy(new_buf, p, old_size);
        return new_buf;
    }

    size_t combined_size = old_size + sizeof(arena_header_t) + next_header->size;
    if (combined_size >= n) {
        header->size = combined_size - sizeof(arena_header_t);
        void *next_ptr = (char *)p + old_size;
        arena_header_t *next_header = (arena_header_t *)next_ptr;
        next_header->size = header->size - old_size - sizeof(arena_header_t);
        return p;
    }

    void *new_buf = arena_alloc(n);
    if (new_buf == NULL) {
        return NULL;
    }
    memcpy(new_buf, p, old_size);
    return new_buf;
}