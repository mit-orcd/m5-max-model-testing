#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGNMENT sizeof(max_align_t)
#define ALIGN_UP(n) (((n) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

struct block {
    size_t size;
    struct block *next;
    int free;
};

static struct {
    void *buf;
    size_t size;
    struct block *free_list;
} state;

void arena_init(void *buf, size_t size) {
    state.buf = buf;
    state.size = size;
    state.free_list = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;

    size_t actual_size = ALIGN_UP(n) + ALIGN_UP(sizeof(struct block));
    if (actual_size > state.size) return NULL;

    struct block *prev = NULL;
    struct block *curr = state.free_list;

    while (curr) {
        if (curr->free && curr->size >= actual_size) {
            if (prev) prev->next = curr->next;
            else state.free_list = curr->next;

            curr->size = actual_size;
            curr->free = 0;
            return (char *)curr + ALIGN_UP(sizeof(struct block));
        }
        prev = curr;
        curr = curr->next;
    }

    if (!state.free_list) {
        struct block *new_block = state.buf;
        new_block->size = state.size;
        new_block->next = NULL;
        new_block->free = 1;
        state.free_list = new_block;
        return arena_alloc(n);
    }

    return NULL;
}

void arena_free(void *p) {
    if (!p) return;

    struct block *block = (struct block *)((char *)p - ALIGN_UP(sizeof(struct block)));
    block->free = 1;

    struct block *curr = state.free_list;
    struct block *prev = NULL;

    while (curr && curr < block) {
        prev = curr;
        curr = curr->next;
    }

    if (prev && prev->free && (char *)prev + ALIGN_UP(prev->size) == (char *)block) {
        prev->size += ALIGN_UP(block->size);
        block = prev;
    }

    if (curr && curr->free && (char *)block + ALIGN_UP(block->size) == (char *)curr) {
        block->size += ALIGN_UP(curr->size);
        block->next = curr->next;
    } else {
        block->next = curr;
    }

    if (!prev) state.free_list = block;
    else prev->next = block;
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }

    struct block *block = (struct block *)((char *)p - ALIGN_UP(sizeof(struct block)));
    void *new_p = p;

    if (block->free) return arena_alloc(n);

    size_t actual_size = ALIGN_UP(n) + ALIGN_UP(sizeof(struct block));
    if (block->size >= actual_size) return p;

    struct block *next = (struct block *)((char *)block + ALIGN_UP(block->size));
    if (next->free && (block->size + ALIGN_UP(next->size)) >= actual_size) {
        block->size += ALIGN_UP(next->size);
        block->next = next->next;
        return p;
    }

    void *new_mem = arena_alloc(n);
    if (!new_mem) return NULL;

    size_t copy_size = (n < block->size - ALIGN_UP(sizeof(struct block))) ? n : (block->size - ALIGN_UP(sizeof(struct block)));
    memcpy(new_mem, p, copy_size);
    arena_free(p);
    return new_mem;
}