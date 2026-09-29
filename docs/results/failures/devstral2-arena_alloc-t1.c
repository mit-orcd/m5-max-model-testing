#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include <string.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN_UP(n) (((n) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))
#define BLOCK_SIZE sizeof(struct Block)
#define MIN_BLOCK_SIZE (BLOCK_SIZE + ALIGNMENT)

struct Block {
    size_t size;
    struct Block *next;
    int used;
};

static struct {
    void *buf;
    size_t size;
    struct Block *free_list;
} arena_state;

void arena_init(void *buf, size_t size) {
    arena_state.buf = buf;
    arena_state.size = size;
    arena_state.free_list = NULL;
}

static void add_to_free_list(struct Block *block) {
    block->next = arena_state.free_list;
    arena_state.free_list = block;
}

static void remove_from_free_list(struct Block *block) {
    struct Block *prev = NULL;
    struct Block *curr = arena_state.free_list;
    while (curr != block) {
        prev = curr;
        curr = curr->next;
    }
    if (prev) {
        prev->next = curr->next;
    } else {
        arena_state.free_list = curr->next;
    }
}

static struct Block *merge_free_blocks(struct Block *block) {
    struct Block *curr = block;
    while (curr->next && !curr->next->used) {
        curr->size += curr->next->size;
        curr->next = curr->next->next;
    }
    return curr;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    n = ALIGN_UP(n);
    struct Block *prev = NULL;
    struct Block *curr = arena_state.free_list;
    while (curr) {
        if (curr->size >= n) {
            if (curr->size >= n + MIN_BLOCK_SIZE) {
                struct Block *new_block = (void *)((uintptr_t)curr + n);
                new_block->size = curr->size - n;
                new_block->used = 0;
                new_block->next = curr->next;
                curr->size = n;
                curr->used = 1;
                if (prev) {
                    prev->next = new_block;
                } else {
                    arena_state.free_list = new_block;
                }
                add_to_free_list(new_block);
            } else {
                curr->used = 1;
                remove_from_free_list(curr);
            }
            return (void *)((uintptr_t)curr + BLOCK_SIZE);
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    struct Block *block = (void *)((uintptr_t)p - BLOCK_SIZE);
    block->used = 0;
    add_to_free_list(block);
    merge_free_blocks(block);
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    n = ALIGN_UP(n);
    struct Block *block = (void *)((uintptr_t)p - BLOCK_SIZE);
    if (block->size >= n) {
        return p;
    }
    struct Block *next_block = (void *)((uintptr_t)block + BLOCK_SIZE + block->size);
    if (!next_block->used && block->size + next_block->size >= n) {
        block->size += next_block->size;
        remove_from_free_list(next_block);
        return p;
    }
    void *new_p = arena_alloc(n);
    if (!new_p) return NULL;
    memcpy(new_p, p, block->size - BLOCK_SIZE);
    arena_free(p);
    return new_p;
}