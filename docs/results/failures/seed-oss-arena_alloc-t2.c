#include <stdalign.h>
#include <stddef.h>
#include <string.h>

typedef struct Block Block;
struct Block {
    size_t size;
    Block* next;
    unsigned char data[];
};

#define BLOCK_HEADER_SIZE offsetof(Block, data)
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void* (*get_arena_buf)(void) = NULL;
static size_t (*get_arena_size)(void) = NULL;

static void* get_arena_buf_impl(void) { return get_arena_buf; }
static size_t get_arena_size_impl(void) { return get_arena_size; }

void arena_init(void* buf, size_t size) {
    if (size < BLOCK_HEADER_SIZE) {
        get_arena_buf = NULL;
        get_arena_size = NULL;
        return;
    }
    get_arena_buf = get_arena_buf_impl;
    get_arena_size = get_arena_size_impl;
    Block* head = (Block*)buf;
    head->size = size - BLOCK_HEADER_SIZE;
    head->next = NULL;
    memset(head->data, 0, head->size);
}

void* arena_alloc(size_t n) {
    if (n == 0 || !get_arena_buf) return NULL;
    size_t align = alignof(max_align_t);
    size_t padding = (align - (n % align)) % align;
    size_t total_size = n + padding;
    Block** prev = &((Block*)get_arena_buf())->next;
    Block* curr = *prev;
    Block* best = NULL;
    size_t best_diff = SIZE_MAX;

    while (curr) {
        size_t diff = curr->size - total_size;
        if (diff >= 0 && diff < best_diff) {
            best = curr;
            best_diff = diff;
        }
        prev = &curr->next;
        curr = curr->next;
    }

    if (!best) return NULL;

    if (best->size >= total_size + BLOCK_HEADER_SIZE + 1) {
        Block* split = (Block*)((unsigned char*)best->data + total_size);
        split->size = best->size - total_size - BLOCK_HEADER_SIZE;
        split->next = best->next;
        best->size = total_size;
        best->next = split;
    }

    memset(best->data, 0, best->size);
    return best->data;
}

void arena_free(void* p) {
    if (!p || !get_arena_buf) return;
    Block* block = (Block*)((unsigned char*)p - BLOCK_HEADER_SIZE);
    Block** prev = &((Block*)get_arena_buf())->next;
    Block* curr = *prev;

    while (curr && curr != block) {
        prev = &curr->next;
        curr = curr->next;
    }

    if (!curr) return;

    if (block->next && !(block->next->size & 1)) {
        block->size += BLOCK_HEADER_SIZE + 1 + block->next->size;
        block->next = block->next->next;
    }

    if (curr->next && !(curr->next->size & 1)) {
        curr->next->size += BLOCK_HEADER_SIZE + 1 + curr->size;
        curr->next->next = curr->next->next->next;
    }

    block->size |= 1;
}

void* arena_realloc(void* p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    if (!get_arena_buf) return NULL;

    Block* block = (Block*)((unsigned char*)p - BLOCK_HEADER_SIZE);
    size_t old_size = block->size & ~1;
    size_t align = alignof(max_align_t);
    size_t padding = (align - (n % align)) % align;
    size_t new_size = n + padding;

    if (new_size <= old_size) {
        if (new_size + BLOCK_HEADER_SIZE + 1 <= old_size) {
            Block* split = (Block*)((unsigned char*)block->data + new_size);
            split->size = old_size - new_size - BLOCK_HEADER_SIZE;
            split->next = block->next;
            block->size = new_size;
            block->next = split;
        }
        return p;
    }

    if (block->next && !(block->next->size & 1)) {
        size_t available = block->next->size & ~1;
        if (available >= new_size - old_size) {
            size_t total_needed = new_size - old_size;
            if (available >= total_needed + BLOCK_HEADER_SIZE + 1) {
                Block* split = (Block*)((unsigned char*)block->next->data + total_needed);
                split->size = block->next->size - total_needed - BLOCK_HEADER_SIZE;
                split->next = block->next->next;
                block->size = (old_size + total_needed) | (block->size & 1);
                block->next = split;
            } else {
                block->size = (old_size + available + BLOCK_HEADER_SIZE + 1) | (block->size & 1);
                block->next = block->next->next;
            }
            return p;
        }
    }

    void* new_p = arena_alloc(n);
    if (new_p) {
        size_t copy_size = MAX(old_size, new_size);
        memcpy(new_p, p, copy_size);
        arena_free(p);
    }
    return new_p;
}