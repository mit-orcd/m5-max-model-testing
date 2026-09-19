#include <stddef.h>
#include <stdalign.h>

typedef struct block {
    size_t size;
    struct block *next;
} block;

typedef struct arena {
    block *head;
    block *tail;
    block *free_list;
} arena;

static inline void *align_up(void *p, size_t alignment) {
    return (void *)(((uintptr_t)p + (alignment - 1)) & ~(alignment - 1));
}

void arena_init(void *buf, size_t size) {
    arena *a = (arena *)buf;
    size_t min_size = sizeof(arena);
    if (size < min_size) {
        a->head = NULL;
        a->tail = NULL;
        a->free_list = NULL;
        return;
    }

    void *raw_block = (void *)((char *)buf + min_size);
    size_t block_size = size - min_size;

    a->head = (block *)raw_block;
    a->head->size = block_size;
    a->head->next = NULL;

    a->tail = a->head;
    a->free_list = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    alignas(max_align_t) char dummy;
    size_t alignment = _Alignof(max_align_t);
    size_t aligned_n = ((n + alignment - 1) & ~(alignment - 1));

    arena *a = (arena *)dummy; 
    block *prev = NULL;
    block *curr = a->free_list;

    while (curr) {
        if (curr->size >= aligned_n) {
            size_t remaining = curr->size - aligned_n;
            if (remaining > 0) {
                block *new_block = (block *)((char *)curr + aligned_n);
                new_block->size = remaining;
                new_block->next = curr->next;
                curr->next = new_block;
                curr->size = aligned_n;
            }
            if (prev) {
                prev->next = curr->next;
            } else {
                a->free_list = curr->next;
            }
            return (void *)(curr + 1);
        }
        prev = curr;
        curr = curr->next;
    }

    curr = a->head;
    prev = a->tail;

    while (curr) {
        if (curr->size >= aligned_n) {
            size_t remaining = curr->size - aligned_n;
            if (remaining > 0) {
                block *new_block = (block *)((char *)curr + aligned_n);
                new_block->size = remaining;
                new_block->next = NULL;
                curr->next = new_block;
                a->tail = new_block;
            }
            if (curr == a->head) {
                a->head = curr->next;
            }
            if (curr == a->tail) {
                a->tail = prev;
            }
            if (prev) {
                prev->next = curr->next;
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
    arena *a = (arena *)p; 
    block *b = (block *)p - 1;
    b->next = a->free_list;
    a->free_list = b;
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!p) {
        return arena_alloc(n);
    }

    alignas(max_align_t) char dummy;
    arena *a = (arena *)dummy;
    block *b = (block *)p - 1;
    size_t old_size = b->size;
    size_t new_size = ((n + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1));

    if (new_size <= old_size) {
        return p;
    }

    block *next = b->next;
    size_t next_size = next ? next->size : 0;
    size_t total_free = next_size;

    if (next && (next->size & 1)) {
        total_free += next->size;
        next = (block *)((char *)next + next->size);
        next_size = next ? next->size : 0;
    }

    if (total_free >= (new_size - old_size)) {
        if (next && (next->size & 1)) {
            block *merged = (block *)((char *)next - next->size);
            merged->size += next_size;
            merged->next = next->next;
            b->next = merged;
        } else {
            b->size += next_size;
            b->next = next->next;
        }
        return p;
    }

    void *new_p = arena_alloc(new_size);
    if (new_p) {
        size_t copy_size = old_size < new_size ? old_size : new_size;
        __builtin_memcpy(new_p, p, copy_size);
        arena_free(p);
    }
    return new_p;
}