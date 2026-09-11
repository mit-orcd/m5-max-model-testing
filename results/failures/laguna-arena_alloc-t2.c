#include <stddef.h>
#include <stdint.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

typedef struct block block_t;
struct block {
    block_t *next;
    size_t size;
};

static struct {
    block_t *head;
    void *buf;
    size_t size;
} arena;

void arena_init(void *buf, size_t size) {
    arena.buf = buf;
    arena.size = size;
    arena.head = (block_t *)buf;
    if (buf) {
        arena.head->next = NULL;
        arena.head->size = size - sizeof(block_t);
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t total = sizeof(block_t) + ALIGN(n);
    block_t *prev = NULL;
    block_t *curr = arena.head;
    while (curr) {
        if (curr->size >= total) {
            if (curr->size > total) {
                block_t *new_block = (block_t *)((char *)curr + total);
                new_block->size = curr->size - total;
                new_block->next = curr->next;
                curr->size = total;
                curr->next = new_block;
            }
            return (void *)(curr + 1);
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    block_t *block = ((block_t *)p) - 1;
    block->next = arena.head;
    arena.head = block;
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
    }
    if (!p) return arena_alloc(n);
    size_t old_size = ((block_t *)p - 1)->size - sizeof(block_t);
    size_t total = sizeof(block_t) + ALIGN(n);
    block_t *block = ((block_t *)p) - 1;
    block_t *next = block->next;
    if (next && next->size > 0) {
        size_t combined = block->size + next->size;
        if (combined >= total) {
            block->size = combined;
            block->next = next->next;
            if (block->size > total) {
                block_t *new_block = (block_t *)((char *)block + total);
                new_block->size = block->size - total;
                new_block->next = block->next;
                block->size = total;
                block->next = new_block;
            }
            return (void *)(block + 1);
        }
    }
    void *new_ptr = arena_alloc(n);
    if (new_ptr) {
        size_t copy_size = (old_size < n) ? old_size : n;
        char *src = (char *)p;
        char *dst = (char *)new_ptr;
        for (size_t i = 0; i < copy_size; i++) dst[i] = src[i];
    }
    return new_ptr;
}