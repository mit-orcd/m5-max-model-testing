#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGNMENT sizeof(max_align_t)
#define ALIGN_UP(n) (((n) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))
#define BLOCK_HEADER_SIZE sizeof(struct BlockHeader)
#define MIN_BLOCK_SIZE (16 + BLOCK_HEADER_SIZE)

struct BlockHeader {
    size_t size;
    struct BlockHeader *next;
    bool is_free;
};

struct Arena {
    struct BlockHeader *free_list;
    void *start;
    size_t capacity;
    size_t used;
};

static struct Arena *arena;
static struct Arena dummy_arena;

void arena_init(void *buf, size_t size) {
    arena = buf;
    arena->free_list = NULL;
    arena->start = buf;
    arena->capacity = size;
    arena->used = 0;
    struct BlockHeader *block = (struct BlockHeader *)buf;
    block->size = size - BLOCK_HEADER_SIZE;
    block->next = NULL;
    block->is_free = true;
    arena->free_list = block;
}

static struct BlockHeader *find_free_block(size_t size) {
    struct BlockHeader *prev = NULL;
    struct BlockHeader *curr = arena->free_list;
    struct BlockHeader *best = NULL;
    size_t best_size = -1;

    while (curr != NULL) {
        if (curr->is_free && curr->size >= size) {
            if (curr->size < best_size || best == NULL) {
                best_size = curr->size;
                best = curr;
                if (best_size == size) break;
            }
        }
        prev = curr;
        curr = curr->next;
    }
    if (best != NULL) {
        if (prev != NULL) prev->next = best->next;
        else arena->free_list = best->next;
        best->next = NULL;
        best->is_free = false;
    }
    return best;
}

static void merge_free_blocks(void) {
    struct BlockHeader *curr = arena->free_list;
    while (curr != NULL && curr->next != NULL) {
        if (curr->is_free && curr->next->is_free) {
            curr->size += BLOCK_HEADER_SIZE + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}

static struct BlockHeader *split_block(struct BlockHeader *block, size_t size) {
    if (block->size > size + MIN_BLOCK_SIZE) {
        struct BlockHeader *new_block = (struct BlockHeader *)((char *)block + BLOCK_HEADER_SIZE + size);
        new_block->size = block->size - size - BLOCK_HEADER_SIZE;
        new_block->next = block->next;
        new_block->is_free = true;
        block->size = size;
        block->next = new_block;
        return new_block;
    }
    return NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t size = ALIGN_UP(n) + BLOCK_HEADER_SIZE;
    struct BlockHeader *block = find_free_block(size);
    if (block == NULL) return NULL;
    struct BlockHeader *split = split_block(block, size);
    if (split != NULL) {
        split->next = arena->free_list;
        arena->free_list = split;
    }
    arena->used += size;
    return (char *)block + BLOCK_HEADER_SIZE;
}

void arena_free(void *p) {
    if (p == NULL) return;
    struct BlockHeader *block = (struct BlockHeader *)((char *)p - BLOCK_HEADER_SIZE);
    block->is_free = true;
    block->next = arena->free_list;
    arena->free_list = block;
    merge_free_blocks();
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    struct BlockHeader *block = (struct BlockHeader *)((char *)p - BLOCK_HEADER_SIZE);
    size_t old_size = block->size;
    size_t new_size = ALIGN_UP(n) + BLOCK_HEADER_SIZE;
    if (new_size <= old_size) return p;
    if (block->next != NULL && block->next->is_free && block->next->size >= new_size - old_size - BLOCK_HEADER_SIZE) {
        block->size = new_size + block->next->size;
        block->next = block->next->next;
        return p;
    }
    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    memcpy(new_p, p, old_size - BLOCK_HEADER_SIZE);
    arena_free(p);
    return new_p;
}