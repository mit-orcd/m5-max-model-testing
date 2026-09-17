#include <stddef.h>
#include <stdint.h>

typedef struct block_header {
    size_t size;
    bool free;
    struct block_header *next;
    struct block_header *prev;
} block_header;

void arena_init(void *buf, size_t size) {
    block_header *header = (block_header *)buf;
    header->size = size - sizeof(block_header);
    header->free = true;
    header->next = NULL;
    header->prev = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;

    block_header *current = (block_header *)((char *)current + sizeof(block_header));
    while (current != NULL) {
        if (current->free && current->size >= n) {
            if (current->size > n) {
                block_header *new_block = (block_header *)((char *)current + sizeof(block_header) + n);
                new_block->size = current->size - n;
                new_block->free = true;
                new_block->next = current->next;
                new_block->prev = current;
                if (current->next != NULL) {
                    current->next->prev = new_block;
                }
                current->size = n;
            }
            current->free = false;
            return (void *)((char *)current + sizeof(block_header));
        }
        current = current->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;

    block_header *block = (block_header *)((char *)p - sizeof(block_header));
    block->free = true;
    if (block->prev != NULL && block->prev->free) {
        block_header *prev_block = block->prev;
        prev_block->size += block->size + sizeof(block_header);
        prev_block->next = block->next;
        if (block->next != NULL) {
            block->next->prev = prev_block;
        }
        block = prev_block;
    }
    if (block->next != NULL && block->next->free) {
        block_header *next_block = block->next;
        block->size += next_block->size + sizeof(block_header);
        block->next = next_block->next;
        if (next_block->next != NULL) {
            next_block->next->prev = block;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    block_header *block = (block_header *)((char *)p - sizeof(block_header));
    if (block->size >= n) {
        if (block->size > n) {
            block_header *new_block = (block_header *)((char *)block + sizeof(block_header) + n);
            new_block->size = block->size - n;
            new_block->free = true;
            new_block->next = block->next;
            new_block->prev = block;
            if (block->next != NULL) {
                block->next->prev = new_block;
            }
            block->size = n;
        }
        return p;
    }

    if (block->next != NULL && block->next->free && (block->size + block->next->size + sizeof(block_header)) >= n) {
        block_header *next_block = block->next;
        block->size += next_block->size + sizeof(block_header);
        block->next = next_block->next;
        if (next_block->next != NULL) {
            next_block->next->prev = block;
        }
        return p;
    }

    void *new_ptr = arena_alloc(n);
    if (new_ptr != NULL) {
        size_t copy_size = (block->size < n) ? block->size : n;
        memcpy(new_ptr, p, copy_size);
        arena_free(p);
        return new_ptr;
    }
    return NULL;
}