#include <stdalign.h>
#include <stddef.h>
#include <string.h>

typedef struct Block Block;
struct Block {
    size_t size;
    Block* next;
    char data[];
};

static Block* head;

void arena_init(void* buf, size_t size) {
    size_t header_size = offsetof(Block, data);
    size_t usable_size = size > header_size ? size - header_size : 0;
    head = (Block*)buf;
    head->size = usable_size;
    head->next = NULL;
    memset(head->data, 0, usable_size);
}

void* arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t align = alignof(max_align_t);
    size_t aligned_n = (n + align - 1) & ~(align - 1);
    size_t header_size = offsetof(Block, data);
    size_t total_size = aligned_n + header_size;

    Block** prev = &head;
    Block* curr = head;

    while (curr) {
        if (curr->size >= aligned_n) {
            if (curr->size - aligned_n >= header_size + align) {
                Block* split = (Block*)((char*)curr + aligned_n);
                split->size = curr->size - aligned_n;
                split->next = curr->next;
                curr->size = aligned_n;
                curr->next = split;
            }
            *prev = curr->next;
            memset(curr->data, 0, aligned_n);
            return curr->data;
        }
        prev = &curr->next;
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void* p) {
    if (!p) return;
    Block* block = (Block*)((char*)p - offsetof(Block, data));
    if (!block->size) return;

    Block** prev = &head;
    Block* curr = head;

    while (curr && curr < block) {
        prev = &curr->next;
        curr = curr->next;
    }

    if (curr == block->next) {
        block->size += curr->size + offsetof(Block, data);
        block->next = curr->next;
    }

    if (prev && (char*)*prev + offsetof(Block, data) + (*prev)->size == (char*)block) {
        Block* prev_block = *prev;
        prev_block->size += block->size + offsetof(Block, data);
        prev_block->next = block->next;
        block = prev_block;
    }

    *prev = block;
    block->next = NULL;
    memset(block->data, 0, block->size);
}

void* arena_realloc(void* p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    Block* block = (Block*)((char*)p - offsetof(Block, data));
    size_t old_size = block->size;
    size_t align = alignof(max_align_t);
    size_t aligned_n = (n + align - 1) & ~(align - 1);
    size_t header_size = offsetof(Block, data);
    size_t total_old = old_size + header_size;

    if (aligned_n <= old_size) {
        memset((char*)p + aligned_n, 0, old_size - aligned_n);
        return p;
    }

    if (block->next && !block->next->next && (char*)block + total_old == (char*)block->next) {
        size_t next_size = block->next->size;
        if (old_size + next_size + header_size >= aligned_n + header_size) {
            size_t new_size = aligned_n;
            size_t remaining = old_size + next_size - new_size;
            if (remaining >= header_size + align) {
                Block* split = (Block*)((char*)block + new_size);
                split->size = remaining;
                split->next = NULL;
                block->next = split;
            } else {
                block->next = NULL;
            }
            block->size = new_size;
            memset((char*)p + old_size, 0, new_size - old_size);
            return p;
        }
    }

    void* new_p = arena_alloc(n);
    if (new_p) {
        size_t copy_size = old_size < aligned_n ? old_size : aligned_n;
        memcpy(new_p, p, copy_size);
        arena_free(p);
    }
    return new_p;
}