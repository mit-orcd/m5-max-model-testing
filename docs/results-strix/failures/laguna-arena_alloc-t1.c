#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    size_t size;
    int free;
    struct block *next;
} block;

static_assert(_Alignof(max_align_t) >= sizeof(block), "Align requirement not met");

static void *align_ptr(void *p, size_t alignment) {
    uintptr_t addr = (uintptr_t)p;
    addr = (addr + alignment - 1) & ~(alignment - 1);
    return (void *)addr;
}

void arena_init(void *buf, size_t size) {
    if (buf == NULL || size < sizeof(block) + _Alignof(max_align_t)) return;
    block *b = (block *)buf;
    b->size = size - sizeof(block);
    b->free = 1;
    b->next = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    block *current = (block *)((char *)arena_init + sizeof(block));
    while (current) {
        if (current->free && current->size >= n) {
            current->free = 0;
            return (void *)(current + 1);
        }
        current = current->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    block *b = (block *)p - 1;
    b->free = 1;
    if (b->next && b->next->free) {
        b->size += sizeof(block) + b->next->size;
        b->next = b->next->next;
    }
    block *prev = (block *)((char *)b - b->size - sizeof(block));
    if (prev->free) {
        prev->size += sizeof(block) + b->size;
        prev->next = b->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (p == NULL) return arena_alloc(n);
    block *b = (block *)p - 1;
    if (b->next && b->next->free && b->size + sizeof(block) + b->next->size >= n) {
        size_t new_size = n;
        if (new_size > b->size) {
            size_t available = sizeof(block) + b->next->size;
            size_t needed = new_size - b->size;
            if (needed > available) needed = available;
            b->size += needed;
            block *next_block = (block *)((char *)b + sizeof(block) + b->size);
            next_block->size = b->size - needed - sizeof(block);
            next_block->free = 1;
            next_block->next = b->next->next;
            b->next = next_block;
        }
        return p;
    }
    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    size_t old_size = b->size;
    size_t copy_size = old_size < n ? old_size : n;
    for (size_t i = 0; i < copy_size; ++i) ((char *)new_p)[i] = ((char *)p)[i];
    arena_free(p);
    return new_p;
}