#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include <string.h>

#define ARENA_ALIGNMENT alignof(max_align_t)

typedef struct {
    size_t size;
    size_t next;
} arena_block_header;

static void *arena_base = NULL;
static size_t arena_size = 0;
static size_t arena_used = 0;

void arena_init(void *buf, size_t size) {
    if (size < sizeof(arena_block_header)) {
        arena_base = NULL;
        arena_size = 0;
        arena_used = 0;
        return;
    }

    arena_base = buf;
    arena_size = size;
    arena_used = 0;

    arena_block_header *header = (arena_block_header *)arena_base;
    header->size = arena_size - sizeof(arena_block_header);
    header->next = 0;
}

void *arena_alloc(size_t n) {
    if (n == 0 || arena_base == NULL) {
        return NULL;
    }

    size_t aligned_n = (n + (ARENA_ALIGNMENT - 1)) & ~(ARENA_ALIGNMENT - 1);
    if (aligned_n < n) {
        return NULL;
    }

    size_t block_size = aligned_n + sizeof(arena_block_header);
    if (block_size > arena_size) {
        return NULL;
    }

    char *current = (char *)arena_base + arena_used;
    arena_block_header *header = (arena_block_header *)current;

    if (arena_used + block_size > arena_size) {
        return NULL;
    }

    header->size = aligned_n;
    header->next = 0;

    arena_used += block_size;
    return (void *)(current + sizeof(arena_block_header));
}

void arena_free(void *p) {
    if (p == NULL || arena_base == NULL) {
        return;
    }

    char *ptr = (char *)p - sizeof(arena_block_header);
    arena_block_header *header = (arena_block_header *)ptr;

    if (header->size == 0 || header->next != 0) {
        return;
    }

    size_t block_size = header->size + sizeof(arena_block_header);
    size_t block_end = arena_used;

    if (block_end == block_size) {
        arena_used -= block_size;
        return;
    }

    char *next_ptr = (char *)arena_base + block_end - sizeof(arena_block_header);
    arena_block_header *next_header = (arena_block_header *)next_ptr;

    if (block_end + block_size == (size_t)next_ptr + next_header->size + sizeof(arena_block_header)) {
        next_header->next = 0;
        arena_used -= block_size;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) {
        return arena_alloc(n);
    }

    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    char *ptr = (char *)p - sizeof(arena_block_header);
    arena_block_header *header = (arena_block_header *)ptr;

    size_t old_size = header->size;
    size_t aligned_n = (n + (ARENA_ALIGNMENT - 1)) & ~(ARENA_ALIGNMENT - 1);
    if (aligned_n < n) {
        return NULL;
    }

    size_t block_size = aligned_n + sizeof(arena_block_header);
    if (block_size > arena_size) {
        return NULL;
    }

    if (header->next == 0) {
        char *current = (char *)arena_base + arena_used;
        if ((size_t)ptr + block_size == (size_t)current) {
            header->size = aligned_n;
            arena_used += (aligned_n - old_size);
            return p;
        }
    }

    void *new_p = arena_alloc(n);
    if (new_p == NULL) {
        return NULL;
    }

    size_t copy_size = (old_size < n) ? old_size : n;
    memcpy(new_p, p, copy_size);

    arena_free(p);
    return new_p;
}