#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

typedef struct Block {
    size_t size;
    struct Block *next;
    int free;
    uint8_t data[];
} Block;

static Block *first_block = NULL;

void arena_init(void *buf, size_t size) {
    if (size < sizeof(Block)) return;
    first_block = (Block *)buf;
    first_block->size = size - sizeof(Block);
    first_block->next = NULL;
    first_block->free = 1;
}

static void merge_blocks(Block **head) {
    Block *current = *head;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += sizeof(Block) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    n = (n + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    Block *current = first_block;
    while (current) {
        if (current->free && current->size >= n) {
            current->size -= n;
            Block *new_block = (Block *)((uint8_t *)current + sizeof(Block) + current->size);
            new_block->size = n;
            new_block->next = current->next;
            new_block->free = 0;
            current->next = new_block;
            return new_block->data;
        }
        current = current->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    Block *block = (Block *)((uint8_t *)p - sizeof(Block));
    block->free = 1;
    merge_blocks(&first_block);
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    n = (n + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    Block *block = (Block *)((uint8_t *)p - sizeof(Block));
    size_t old_size = block->size;
    if (old_size >= n) return p;

    Block *next_block = block->next;
    if (next_block && next_block->free && sizeof(Block) + next_block->size + old_size >= n) {
        block->size = old_size + sizeof(Block) + next_block->size;
        block->next = next_block->next;
        return p;
    }

    void *new_p = arena_alloc(n);
    if (!new_p) return NULL;
    for (size_t i = 0; i < old_size && i < n; i++) {
        ((uint8_t *)new_p)[i] = ((uint8_t *)p)[i];
    }
    arena_free(p);
    return new_p;
}