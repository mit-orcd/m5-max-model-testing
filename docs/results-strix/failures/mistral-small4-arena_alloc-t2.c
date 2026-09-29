#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

#define ARENA_ALIGNMENT _Alignof(max_align_t)

typedef struct {
    size_t size;
    char data[];
} arena_header;

typedef struct {
    size_t size;
    size_t prev_free;
} arena_block;

static char *arena_base = NULL;
static size_t arena_size = 0;
static arena_block *first_block = NULL;
static arena_block *last_block = NULL;

void arena_init(void *buf, size_t size) {
    if (size < sizeof(arena_block)) {
        arena_base = NULL;
        arena_size = 0;
        first_block = NULL;
        last_block = NULL;
        return;
    }

    alignas(ARENA_ALIGNMENT) char *aligned_buf = (char *)buf;
    if ((uintptr_t)aligned_buf % ARENA_ALIGNMENT != 0) {
        aligned_buf = (char *)buf + (ARENA_ALIGNMENT - ((uintptr_t)buf % ARENA_ALIGNMENT));
        size -= (aligned_buf - (char *)buf);
    }

    arena_header *header = (arena_header *)aligned_buf;
    arena_block *block = (arena_block *)(aligned_buf + sizeof(arena_header));

    block->size = size - sizeof(arena_header);
    block->prev_free = 0;

    first_block = block;
    last_block = block;

    header->size = size;
    arena_base = aligned_buf;
    arena_size = size;
}

void *arena_alloc(size_t n) {
    if (n == 0 || arena_base == NULL) {
        return NULL;
    }

    size_t total_size = n + sizeof(arena_block);
    size_t aligned_size = total_size;
    if (aligned_size % ARENA_ALIGNMENT != 0) {
        aligned_size += (ARENA_ALIGNMENT - (aligned_size % ARENA_ALIGNMENT));
    }

    arena_block *block = first_block;
    while (block != NULL) {
        if (block->size >= aligned_size) {
            if (block->size > aligned_size + sizeof(arena_block)) {
                arena_block *next_block = (arena_block *)((char *)block + sizeof(arena_block) + block->size);
                next_block->size = block->size - aligned_size - sizeof(arena_block);
                next_block->prev_free = 0;
                next_block->prev_free = 1;

                last_block = next_block;
                block->size = aligned_size;
            }

            char *next = (char *)block + sizeof(arena_block) + block->size;
            char *next_block = (char *)block + sizeof(arena_block) + block->size;

            if (next < (char *)arena_base + arena_size && ((arena_block *)next)->prev_free) {
                arena_block *merge_block = (arena_block *)next;
                block->size += sizeof(arena_block) + merge_block->size;

                if (merge_block == last_block) {
                    last_block = block;
                }

                merge_block = (arena_block *)((char *)merge_block + sizeof(arena_block) + merge_block->size);
                if (merge_block < (arena_block *)((char *)arena_base + arena_size)) {
                    merge_block->prev_free = 1;
                }
            } else {
                block->prev_free = 0;
            }

            return (void *)((char *)block + sizeof(arena_block));
        }

        char *next = (char *)block + sizeof(arena_block) + block->size;
        if (next >= (char *)arena_base + arena_size) {
            return NULL;
        }

        if (block->size == 0) {
            block->prev_free = 1;
        }

        block = (arena_block *)next;
    }

    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) {
        return;
    }

    arena_block *block = (arena_block *)((char *)p - sizeof(arena_block));
    if ((char *)block < arena_base || (char *)block >= arena_base + arena_size) {
        return;
    }

    block->prev_free = 1;

    char *prev = (char *)block - sizeof(arena_block);
    if (prev >= arena_base && ((arena_block *)prev)->prev_free) {
        arena_block *prev_block = (arena_block *)prev;
        if (prev_block->size + sizeof(arena_block) + block->size > ((arena_block *)arena_base + arena_size)->size) {
            return;
        }

        prev_block->size += sizeof(arena_block) + block->size;
        prev_block->prev_free = 0;

        if (block == last_block) {
            last_block = prev_block;
        }

        block = prev_block;
    }

    char *next = (char *)block + sizeof(arena_block) + block->size;
    if (next < arena_base + arena_size && ((arena_block *)next)->prev_free) {
        arena_block *next_block = (arena_block *)next;
        if (next_block->size + sizeof(arena_block) + block->size > ((arena_block *)arena_base + arena_size)->size) {
            return;
        }

        block->size += sizeof(arena_block) + next_block->size;
        if (next_block == last_block) {
            last_block = block;
        }

        next = (char *)block + sizeof(arena_block) + block->size;
        if (next < arena_base + arena_size) {
            ((arena_block *)next)->prev_free = 0;
        }
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

    arena_block *block = (arena_block *)((char *)p - sizeof(arena_block);
    if ((char *)block < arena_base || (char *)block >= arena_base + arena_size) {
        return NULL;
    }

    size_t total_size = n + sizeof(arena_block);
    size_t aligned_size = total_size;
    if (aligned_size % ARENA_ALIGNMENT != 0) {
        aligned_size += (ARENA_ALIGNMENT - (aligned_size % ARENA_ALIGNMENT));
    }

    if (block->size >= aligned_size) {
        return p;
    }

    char *next = (char *)block + sizeof(arena_block) + block->size;
    if (next < arena_base + arena_size && ((arena_block *)next)->prev_free) {
        arena_block *next_block = (arena_block *)next;
        size_t merge_size = block->size + sizeof(arena_block) + next_block->size;

        if (merge_size >= aligned_size) {
            block->size = merge_size;

            if (next_block == last_block) {
                last_block = block;
            }

            next = (char *)block + sizeof(arena_block) + block->size;
            if (next < arena_base + arena_size) {
                ((arena_block *)next)->prev_free = 0;
            }

            return p;
        }
    }

    void *new_p = arena_alloc(n);
    if (new_p == NULL) {
        return NULL;
    }

    size_t old_size = block->size - sizeof(arena_block);
    size_t copy_size = old_size < n ? old_size : n;
    memcpy(new_p, p, copy_size);

    arena_free(p);
    return new_p;
}