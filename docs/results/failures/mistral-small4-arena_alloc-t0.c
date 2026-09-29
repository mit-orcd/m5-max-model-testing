#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

#define ARENA_ALIGNMENT alignof(max_align_t)

typedef struct {
    size_t size;
    size_t used;
    char data[];
} arena_header;

static arena_header *header = NULL;

void arena_init(void *buf, size_t size) {
    if (size < sizeof(arena_header)) {
        header = NULL;
        return;
    }
    header = (arena_header *)buf;
    header->size = size - sizeof(arena_header);
    header->used = 0;
}

void *arena_alloc(size_t n) {
    if (n == 0 || header == NULL) return NULL;

    size_t aligned_n = (n + (ARENA_ALIGNMENT - 1)) & ~(ARENA_ALIGNMENT - 1);
    if (header->used + aligned_n > header->size) return NULL;

    void *p = (void *)(header->data + header->used);
    header->used += aligned_n;
    return p;
}

void arena_free(void *p) {
    if (p == NULL || header == NULL) return;

    char *block = (char *)p;
    if (block < header->data || block >= header->data + header->size) return;

    size_t offset = block - header->data;
    if (offset == header->used) {
        header->used = 0;
        return;
    }

    if (header->used == 0) return;

    size_t aligned_n = (header->used - offset + (ARENA_ALIGNMENT - 1)) & ~(ARENA_ALIGNMENT - 1);
    if (aligned_n == 0) return;

    if (offset + aligned_n == header->used) {
        header->used = offset;
        return;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (header == NULL) return NULL;

    if (p == NULL) {
        return arena_alloc(n);
    }

    char *block = (char *)p;
    if (block < header->data || block >= header->data + header->size) return NULL;

    size_t offset = block - header->data;
    if (header->used == 0 || offset >= header->used) return NULL;

    size_t old_size = header->used - offset;
    size_t aligned_n = (n + (ARENA_ALIGNMENT - 1)) & ~(ARENA_ALIGNMENT - 1);

    if (aligned_n <= old_size) {
        return p;
    }

    if (offset + aligned_n > header->size) {
        void *new_p = arena_alloc(n);
        if (new_p == NULL) return NULL;
        memcpy(new_p, p, old_size);
        arena_free(p);
        return new_p;
    }

    if (offset + old_size < header->size) {
        char *next_block = header->data + offset + old_size;
        size_t next_offset = next_block - header->data;
        if (next_offset < header->used && header->used - next_offset > 0) {
            size_t next_size = header->used - next_offset;
            if (aligned_n <= next_size) {
                header->used = offset + aligned_n;
                return p;
            }
        }
    }

    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    memcpy(new_p, p, old_size);
    arena_free(p);
    return new_p;
}