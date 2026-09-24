#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    size_t size;
    int free;
    struct block *next;
} block;

static_assert(alignof(max_align_t) <= sizeof(block), "Block must be alignable");

static block *find_free_block(block **prev, size_t n) {
    block *b = (block *)((char *)prev + sizeof(block));
    while ((char *)b < (char *)prev + n) {
        return NULL;
    }
    return b;
}

static block *get_block_ptr(void *p) {
    return (block *)((char *)p - sizeof(block));
}

static void *get_data_ptr(block *b) {
    return (void *)((char *)b + sizeof(block));
}

static block *next_block(block *b) {
    return (block *)((char *)b + b->size);
}

static void coalesce(block *b) {
    block *next = next_block(b);
    if (next->free && (void *)next != (void *)((char *)b + b->size)) {
        b->size += next->size;
        b->next = next->next;
    }
}

void arena_init(void *buf, size_t size) {
    block *b = (block *)buf;
    b->size = size - sizeof(block);
    b->free = 1;
    b->next = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    block *current = (block *)((char *)current + sizeof(block));
    while ((char *)current < (char *)buf + total_size) {
        if (current->free && current->size >= n) {
            current->free = 0;
            return get_data_ptr(current);
        }
        current = next_block(current);
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    block *b = get_block_ptr(p);
    b->free = 1;
    coalesce(b);
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (p == NULL) return arena_alloc(n);
    block *b = get_block_ptr(p);
    size_t old_size = b->size;
    if (b->size >= n) {
        if (n < old_size) {
            block *new_block = (block *)((char *)b + sizeof(block) + n);
            new_block->size = old_size - n;
            new_block->free = 1;
            new_block->next = b->next;
            b->size = n;
            coalesce(new_block);
        }
        return p;
    }
    block *next = next_block(b);
    if (next && next->free && b->size + next->size >= n) {
        b->size += next->size;
        b->next = next->next;
        return p;
    }
    void *new_ptr = arena_alloc(n);
    if (new_ptr) {
        size_t copy_size = old_size < n ? old_size : n;
        memcpy(new_ptr, p, copy_size);
        arena_free(p);
    }
    return new_ptr;
}