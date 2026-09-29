#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

#define BLOCK_HEADER_SIZE ((sizeof(struct block_header) + (alignof(max_align_t) - 1)) & ~(alignof(max_align_t) - 1))

struct block_header {
    size_t size;
    struct block_header *next;
    struct block_header *prev;
};

struct arena_state {
    struct block_header *head;
    struct block_header *tail;
};

static struct arena_state arena;

static void *align_up(void *ptr, size_t alignment) {
    return (void *)(((uintptr_t)ptr + (alignment - 1)) & ~((uintptr_t)(alignment - 1)));
}

static size_t align_up_size(size_t size, size_t alignment) {
    return (size + (alignment - 1)) & ~((size_t)(alignment - 1));
}

void arena_init(void *buf, size_t size) {
    if (!buf || size < BLOCK_HEADER_SIZE) {
        return;
    }

    struct block_header *block = (struct block_header *)align_up(buf, alignof(max_align_t));
    size_t usable_size = size - ((char *)block - (char *)buf);

    if (usable_size < BLOCK_HEADER_SIZE) {
        return;
    }

    block->size = usable_size - BLOCK_HEADER_SIZE;
    block->next = NULL;
    block->prev = NULL;

    arena.head = block;
    arena.tail = block;
}

void *arena_alloc(size_t n) {
    if (n == 0) {
        return NULL;
    }

    size_t aligned_n = align_up_size(n, alignof(max_align_t));

    struct block_header *current = arena.head;
    while (current != NULL) {
        if (current->size >= aligned_n) {
            if (current->size >= aligned_n + BLOCK_HEADER_SIZE) {
                // Split the block
                struct block_header *new_block = (struct block_header *)((char *)current + BLOCK_HEADER_SIZE + aligned_n);
                new_block->size = current->size - aligned_n - BLOCK_HEADER_SIZE;
                new_block->next = current->next;
                new_block->prev = current;

                if (current->next) {
                    current->next->prev = new_block;
                } else {
                    arena.tail = new_block;
                }

                current->next = new_block;
                current->size = aligned_n;
            }

            return (void *)((char *)current + BLOCK_HEADER_SIZE);
        }
        current = current->next;
    }

    return NULL;
}

static void remove_block(struct block_header *block) {
    if (block->prev) {
        block->prev->next = block->next;
    } else {
        arena.head = block->next;
    }

    if (block->next) {
        block->next->prev = block->prev;
    } else {
        arena.tail = block->prev;
    }

    block->next = NULL;
    block->prev = NULL;
}

static void coalesce(struct block_header *block) {
    // Coalesce with next free block
    while (block->next && ((char *)block->next == (char *)block + BLOCK_HEADER_SIZE + block->size)) {
        struct block_header *next = block->next;
        block->size += BLOCK_HEADER_SIZE + next->size;
        remove_block(next);
    }

    // Coalesce with previous free block
    while (block->prev && ((char *)block == (char *)block->prev + BLOCK_HEADER_SIZE + block->prev->size)) {
        struct block_header *prev = block->prev;
        prev->size += BLOCK_HEADER_SIZE + block->size;
        remove_block(block);
        block = prev;
    }
}

void arena_free(void *p) {
    if (!p) {
        return;
    }

    struct block_header *block = (struct block_header *)((char *)p - BLOCK_HEADER_SIZE);
    coalesce(block);
}

void *arena_realloc(void *p, size_t n) {
    if (!p) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    struct block_header *block = (struct block_header *)((char *)p - BLOCK_HEADER_SIZE);
    size_t aligned_n = align_up_size(n, alignof(max_align_t));

    if (aligned_n <= block->size) {
        return p;
    }

    // Check if the next block is free and can be used to grow in place
    if (block->next && ((char *)block->next == (char *)block + BLOCK_HEADER_SIZE + block->size)) {
        size_t available = block->size + BLOCK_HEADER_SIZE + block->next->size;
        if (available >= aligned_n) {
            // Grow into the next block
            struct block_header *next = block->next;
            size_t remaining = available - aligned_n;

            if (remaining >= BLOCK_HEADER_SIZE) {
                // Split the next block
                struct block_header *new_block = (struct block_header *)((char *)block + BLOCK_HEADER_SIZE + aligned_n);
                new_block->size = remaining - BLOCK_HEADER_SIZE;
                new_block->next = next->next;
                new_block->prev = block;

                if (next->next) {
                    next->next->prev = new_block;
                } else {
                    arena.tail = new_block;
                }

                block->next = new_block;
                block->size = aligned_n;
            } else {
                // Merge entirely
                block->size = available;
                remove_block(next);
            }

            return p;
        }
    }

    // Need to allocate a new block and copy data
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) {
        return NULL;
    }

    size_t copy_size = block->size < aligned_n ? block->size : aligned_n;
    char *src = (char *)p;
    char *dst = (char *)new_ptr;
    for (size_t i = 0; i < copy_size; ++i) {
        dst[i] = src[i];
    }

    arena_free(p);
    return new_ptr;
}