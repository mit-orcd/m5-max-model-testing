#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define ALIGNMENT sizeof(max_align_t)
#define ALIGN_UP(n) (((n) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))
#define BLOCK_HEADER_SIZE (sizeof(struct block_header))
#define MIN_BLOCK_SIZE (BLOCK_HEADER_SIZE + ALIGNMENT)

struct block_header {
    size_t size;
    struct block_header *next;
    unsigned is_free : 1;
};

static struct {
    void *buf;
    size_t buf_size;
    struct block_header *free_list;
} state;

void arena_init(void *buf, size_t size) {
    if (size < MIN_BLOCK_SIZE) return;
    state.buf = buf;
    state.buf_size = size;
    state.free_list = (struct block_header *)buf;
    state.free_list->size = size - BLOCK_HEADER_SIZE;
    state.free_list->next = NULL;
    state.free_list->is_free = 1;
}

static void merge_free_blocks(struct block_header *block) {
    if (block->next && block->next->is_free) {
        block->size += BLOCK_HEADER_SIZE + block->next->size;
        block->next = block->next->next;
    }
    if (block != state.free_list) {
        struct block_header *prev = (struct block_header *)((uint8_t *)block - BLOCK_HEADER_SIZE);
        if (prev->is_free) {
            prev->size += BLOCK_HEADER_SIZE + block->size;
            prev->next = block->next;
            block = prev;
        }
    }
    state.free_list = block;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    n = ALIGN_UP(n);
    struct block_header *prev = NULL;
    for (struct block_header *curr = state.free_list; curr; prev = curr, curr = curr->next) {
        if (curr->size >= n) {
            if (curr->size >= n + MIN_BLOCK_SIZE) {
                struct block_header *new_block = (struct block_header *)((uint8_t *)curr + BLOCK_HEADER_SIZE + n);
                new_block->size = curr->size - n - BLOCK_HEADER_SIZE;
                new_block->next = curr->next;
                new_block->is_free = 1;
                curr->size = n;
                curr->next = new_block;
            }
            curr->is_free = 0;
            if (prev) prev->next = curr;
            else state.free_list = curr->next;
            return (uint8_t *)curr + BLOCK_HEADER_SIZE;
        }
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    struct block_header *block = (struct block_header *)((uint8_t *)p - BLOCK_HEADER_SIZE);
    block->is_free = 1;
    merge_free_blocks(block);
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    n = ALIGN_UP(n);
    struct block_header *block = (struct block_header *)((uint8_t *)p - BLOCK_HEADER_SIZE);
    if (!block->is_free && block->size >= n) {
        if (block->next && block->next->is_free && block->size + BLOCK_HEADER_SIZE + block->next->size >= n) {
            block->size += BLOCK_HEADER_SIZE + block->next->size;
            block->next = block->next->next;
        }
        return p;
    }
    void *new_p = arena_alloc(n);
    if (new_p) {
        size_t copy_size = (n < block->size) ? n : block->size;
        memcpy(new_p, p, copy_size);
        arena_free(p);
    }
    return new_p;
}