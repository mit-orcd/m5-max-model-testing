#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

static char *arena_base;
static size_t arena_size;
static char *arena_top;

typedef struct block {
    size_t size;
    int is_free;
    struct block *next;
    struct block *prev;
    char data[];
} block_t;

#define HEADER_SIZE (sizeof(block_t))
#define ALIGNMENT (alignof(max_align_t))
#define BLOCK_ALIGN(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define HEADER_ALIGN(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

void arena_init(void *buf, size_t size) {
    if (!buf || size < HEADER_ALIGN(HEADER_SIZE) + ALIGNMENT) {
        arena_base = NULL;
        arena_size = 0;
        arena_top = NULL;
        return;
    }

    arena_base = (char *)buf;
    arena_size = size;

    size_t header_space = HEADER_ALIGN(HEADER_SIZE);
    if (arena_size < header_space + ALIGNMENT) {
        arena_base = NULL;
        arena_size = 0;
        arena_top = NULL;
        return;
    }

    block_t *first = (block_t *)arena_base;
    first->size = arena_size - header_space;
    first->is_free = 1;
    first->prev = NULL;
    first->next = NULL;

    arena_top = arena_base + arena_size;
}

void *arena_alloc(size_t n) {
    if (!arena_base || n == 0) {
        return NULL;
    }

    size_t needed = BLOCK_ALIGN(n);
    size_t total_needed = HEADER_ALIGN(HEADER_SIZE) + needed;

    if (total_needed > arena_size) {
        return NULL;
    }

    block_t *current = (block_t *)arena_base;

    while (current) {
        if (current->is_free && current->size >= total_needed) {
            size_t remaining = current->size - total_needed;

            if (remaining >= HEADER_ALIGN(HEADER_SIZE) + ALIGNMENT) {
                block_t *new_block = (block_t *)((char *)current + total_needed);
                new_block->size = remaining - HEADER_ALIGN(HEADER_SIZE);
                new_block->is_free = 1;
                new_block->prev = current;
                new_block->next = current->next;

                if (current->next) {
                    current->next->prev = new_block;
                }
                current->next = new_block;
                current->size = HEADER_ALIGN(HEADER_SIZE);
            } else {
                current->size = total_needed;
            }

            current->is_free = 0;
            return (void *)current->data;
        }
        current = current->next;
    }

    return NULL;
}

void arena_free(void *p) {
    if (!p) return;

    block_t *block = (block_t *)((char *)p - HEADER_ALIGN(HEADER_SIZE));

    if (block < (block_t *)arena_base || (char *)block >= arena_top) {
        return;
    }

    if (block->is_free) return;

    block->is_free = 1;

    block_t *next = block->next;
    if (next && next->is_free) {
        block->size += HEADER_ALIGN(HEADER_SIZE) + next->size;
        block->next = next->next;
        if (next->next) {
            next->next->prev = block;
        }
    }

    block_t *prev = block->prev;
    if (prev && prev->is_free) {
        prev->size += HEADER_ALIGN(HEADER_SIZE) + block->size;
        prev->next = block->next;
        if (block->next) {
            block->next->prev = prev;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    block_t *block = (block_t *)((char *)p - HEADER_ALIGN(HEADER_SIZE));

    if (block < (block_t *)arena_base || (char *)block >= arena_top || !block->is_free) {
        return NULL;
    }

    size_t old_data_size = block->size - HEADER_ALIGN(HEADER_SIZE);
    size_t needed = BLOCK_ALIGN(n);
    size_t total_needed = HEADER_ALIGN(HEADER_SIZE) + needed;

    if (old_data_size >= needed) {
        return p;
    }

    if (block->next && block->next->is_free) {
        size_t combined = block->size + HEADER_ALIGN(HEADER_SIZE) + block->next->size;
        if (combined >= total_needed) {
            block_t *next = block->next;
            block->size = combined;
            block->next = next->next;
            if (next->next) {
                next->next->prev = block;
            }

            size_t remaining = block->size - total_needed;
            if (remaining >= HEADER_ALIGN(HEADER_SIZE) + ALIGNMENT) {
                block_t *new_block = (block_t *)((char *)block + total_needed);
                new_block->size = remaining - HEADER_ALIGN(HEADER_SIZE);
                new_block->is_free = 1;
                new_block->prev = block;
                new_block->next = block->next;

                if (block->next) {
                    block->next->prev = new_block;
                }
                block->next = new_block;
                block->size = total_needed;
            }
            return p;
        }
    }

    void *new_ptr = arena_alloc(n);
    if (new_ptr) {
        size_t copy_size = (old_data_size < needed) ? old_data_size : needed;
        __builtin_memcpy(new_ptr, p, copy_size);
        arena_free(p);
    }

    return new_ptr;
}