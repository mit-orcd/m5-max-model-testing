#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ARENA_MAGIC 0xA110CA7E
#define ARENA_FREE_MAGIC 0xF331C7F1

typedef struct arena_block {
    uint32_t magic;
    size_t size;
    struct arena_block *next;
    struct arena_block *prev;
} arena_block_t;

static arena_block_t *arena_head = NULL;

static void *align_ptr(void *p, size_t alignment) {
    uintptr_t addr = (uintptr_t)p;
    uintptr_t aligned = (addr + alignment - 1) & ~(alignment - 1);
    return (void *)aligned;
}

static void coalesce(arena_block_t *block) {
    if (block->next && block->next->magic == ARENA_FREE_MAGIC) {
        arena_block_t *next = block->next;
        block->size += sizeof(arena_block_t) + next->size;
        block->next = next->next;
        if (next->next) {
            next->next->prev = block;
        }
        block->magic = ARENA_FREE_MAGIC;
    }
}

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(arena_block_t)) return;
    arena_block_t *block = (arena_block_t *)buf;
    block->magic = ARENA_FREE_MAGIC;
    block->size = size - sizeof(arena_block_t);
    block->next = NULL;
    block->prev = NULL;
    arena_head = block;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t aligned_n = (n + sizeof(arena_block_t) - 1) & ~(sizeof(arena_block_t) - 1);
    aligned_n = (aligned_n + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    
    arena_block_t *current = arena_head;
    while (current) {
        if (current->magic == ARENA_FREE_MAGIC && current->size >= aligned_n) {
            arena_block_t *new_block = current;
            size_t remaining = current->size - aligned_n;
            if (remaining >= sizeof(arena_block_t) + _Alignof(max_align_t)) {
                arena_block_t *split = (arena_block_t *)((char *)current + sizeof(arena_block_t) + aligned_n);
                split->magic = ARENA_FREE_MAGIC;
                split->size = remaining - sizeof(arena_block_t);
                split->next = current->next;
                split->prev = current;
                if (current->next) {
                    current->next->prev = split;
                }
                current->next = split;
                current->size = aligned_n;
            }
            current->magic = ARENA_MAGIC;
            return (char *)current + sizeof(arena_block_t);
        }
        current = current->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    arena_block_t *block = (arena_block_t *)((char *)p - sizeof(arena_block_t));
    block->magic = ARENA_FREE_MAGIC;
    coalesce(block);
    if (block->prev) {
        coalesce(block->prev);
    }
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!p) {
        return arena_alloc(n);
    }
    
    arena_block_t *block = (arena_block_t *)((char *)p - sizeof(arena_block_t));
    if (block->magic != ARENA_MAGIC) {
        return NULL;
    }
    
    size_t new_aligned_n = (n + sizeof(arena_block_t) - 1) & ~(sizeof(arena_block_t) - 1);
    new_aligned_n = (new_aligned_n + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    
    if (block->size >= new_aligned_n) {
        if (block->next && block->next->magic == ARENA_FREE_MAGIC) {
            size_t combined_size = block->size + sizeof(arena_block_t) + block->next->size;
            if (combined_size >= new_aligned_n) {
                block->size = combined_size - sizeof(arena_block_t);
                block->next = block->next->next;
                if (block->next) {
                    block->next->prev = block;
                }
                return p;
            }
        }
        return p;
    }
    
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) return NULL;
    
    size_t copy_size = n < block->size ? n : block->size;
    memcpy(new_ptr, p, copy_size);
    arena_free(p);
    return new_ptr;
}