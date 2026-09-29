#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGNMENT (alignof(max_align_t))
#define ALIGN_UP(x) (((x) + (ALIGNMENT) - 1) & ~((ALIGNMENT) - 1))

typedef struct Block {
    size_t size;
    int free;
    struct Block *next;
} Block;

static Block *arena_base = NULL;
static size_t arena_size = 0;

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(Block)) {
        arena_base = NULL;
        arena_size = 0;
        return;
    }
    arena_base = (Block *)buf;
    arena_size = size;
    arena_base->size = size - sizeof(Block);
    arena_base->free = 1;
    arena_base->next = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0 || !arena_base) return NULL;
    size_t aligned_n = ALIGN_UP(n);
    Block *curr = arena_base;
    while (curr) {
        if (curr->free && curr->size >= aligned_n) {
            if (curr->size >= aligned_n + sizeof(Block)) {
                Block *new_block = (Block *)((char *)curr + sizeof(Block) + aligned_n);
                new_block->size = curr->size - aligned_n - sizeof(Block);
                new_block->free = 1;
                new_block->next = curr->next;
                curr->size = aligned_n;
                curr->next = new_block;
            } else {
                curr->next = NULL;
            }
            curr->free = 0;
            return (void *)((char *)curr + sizeof(Block));
        }
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p || !arena_base) return;
    Block *block = (Block *)((char *)p - sizeof(Block));
    block->free = 1;
    Block *curr = arena_base;
    while (curr) {
        if (curr->free && curr->next && curr->next->free) {
            curr->size += sizeof(Block) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!p) return arena_alloc(n);
    
    Block *block = (Block *)((char *)p - sizeof(Block));
    size_t old_size = block->size;
    size_t aligned_n = ALIGN_UP(n);
    
    if (aligned_n <= old_size) {
        if (aligned_n + sizeof(Block) <= old_size) {
            size_t new_block_size = old_size - aligned_n - sizeof(Block);
            Block *new_block = (Block *)((char *)block + sizeof(Block) + aligned_n);
            new_block->size = new_block_size;
            new_block->free = 1;
            new_block->next = block->next;
            block->size = aligned_n;
            block->next = new_block;
            Block *curr = arena_base;
            while (curr) {
                if (curr->free && curr->next && curr->next->free) {
                    curr->size += sizeof(Block) + curr->next->size;
                    curr->next = curr->next->next;
                } else {
                    curr = curr->next;
                }
            }
        }
        return p;
    }
    
    Block *next = block->next;
    if (next && next->free && block->size + sizeof(Block) + next->size >= aligned_n) {
        size_t new_size = block->size + sizeof(Block) + next->size;
        size_t remaining = new_size - aligned_n;
        block->size = aligned_n;
        if (remaining >= sizeof(Block)) {
            Block *new_block = (Block *)((char *)block + sizeof(Block) + aligned_n);
            new_block->size = remaining - sizeof(Block);
            new_block->free = 1;
            new_block->next = next->next;
            block->next = new_block;
        } else {
            block->next = next->next;
        }
        return p;
    }
    
    void *new_p = arena_alloc(n);
    if (!new_p) return NULL;
    size_t copy_size = old_size < n ? old_size : n;
    memcpy(new_p, p, copy_size);
    arena_free(p);
    return new_p;
}