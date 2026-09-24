#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include <string.h>

typedef struct {
    size_t size;
    uint8_t data[];
} arena_header;

#define MIN_ALIGNMENT alignof(max_align_t)
#define MAX_ALIGNMENT alignof(max_align_t)

static arena_header *arena = NULL;
static size_t arena_size = 0;

void arena_init(void *buf, size_t size) {
    if (buf == NULL || size < sizeof(arena_header)) {
        arena = NULL;
        arena_size = 0;
        return;
    }

    arena = (arena_header *)buf;
    arena->size = size - sizeof(arena_header);
}

void *arena_alloc(size_t n) {
    if (arena == NULL || n == 0 || n > arena->size) {
        return NULL;
    }

    void *ptr = (void *)((uintptr_t)(arena + 1) + arena->size - n);
    arena->size -= n;
    return ptr;
}

void arena_free(void *p) {
    if (p == NULL || arena == NULL) {
        return;
    }

    arena_header *header = arena;
    if ((uintptr_t)p >= (uintptr_t)header && (uintptr_t)p < (uintptr_t)(header + 1) + header->size) {
        size_t offset = (size_t)((uintptr_t)p - (uintptr_t)(header + 1));
        header->size += offset;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (arena == NULL || n == 0) {
        return NULL;
    }

    if (p == NULL) {
        return arena_alloc(n);
    }

    arena_header *header = arena;
    void *base = header + 1;

    if ((uintptr_t)p < (uintptr_t)base || (uintptr_t)p >= (uintptr_t)base + header->size) {
        return NULL;
    }

    size_t old_size = header->size;
    size_t old_offset = (size_t)((uintptr_t)p - (uintptr_t)base);

    if (old_offset + n <= old_size) {
        return p;
    }

    void *new_ptr = arena_alloc(n);
    if (new_ptr == NULL) {
        return NULL;
    }

    size_t copy_size = (n < old_size) ? n : old_size;
    memcpy(new_ptr, p, copy_size);

    arena_header *new_header = (arena_header *)((uint8_t *)base - old_size + n);
    new_header->size = old_size - old_offset - copy_size;
    header->size = old_offset;

    return new_ptr;
}