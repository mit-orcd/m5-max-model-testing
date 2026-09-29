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
    void *end;
    void *buf;
    size_t size;
} arena;

void arena_init(void *buf, size_t size) {
    arena.buf = buf;
    arena.size = size;
    arena.end = (char *)buf + size;
    if (size >= sizeof(block_t) + ALIGNMENT) {
        block_t *b = (block_t *)((char *)buf + ALIGN(sizeof(block_t)));
        b->next = NULL;
        b->size = size - ALIGN(sizeof(block_t)) - ALIGNment;
        arena.head = b;
    } else {
        arena.head = NULL;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t total = ALIGN(sizeof(block_t)) + ALIGN(n);
    block_t *prev = NULL;
    block_t *curr = arena.head;
    while (curr) {
        if (curr->size >= total) {
            if (curr->size > total) {
                block_t *new_block = (block_t *)((char *)curr + total);
                new_block->next = curr->next;
                new_block->size = curr->size - total;
                curr->next = new_block;
                curr->size = total;
            }
            void *p = (char *)curr + ALIGN(sizeof(block_t));
            curr->size = total;
            return p;
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    block_t *b = (block_t *)((char *)p - ALIGN(sizeof(block_t)));
    b->next = arena.head;
    arena.head = b;
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!p) return arena_alloc(n);
    size_t old_size = *(size_t *)((char *)p - ALIGN(sizeof(block_t)));
    size_t new_total = ALIGN(sizeof(block_t)) + ALIGN(n);
    block_t *b = (block_t *)((char *)p - ALIGN(sizeof(block_t)));
    block_t *next = b->next;
    if (next && next->size >= new_total - old_size) {
        size_t merged_size = old_size + next->size;
        b->size = merged_size;
        b->next = next->next;
        return p;
    }
    void *new_p = arena_alloc(n);
    if (new_P) {
        size_t copy_size = (old_size < n) ? old_size : n;
        char *dst = new_p;
        char *src = p;
        for (size_t i = 0; i < copy_size; i++) dst[i] = src[i];
        arena_free(p);
    }
    return new_P;
}