#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include <string.h>

#define ARENA_ALIGNMENT alignof(max_align_t)

typedef struct {
    size_t size;
    struct block *next;
} block_header;

typedef struct block {
    size_t size;
    struct block *next;
    char data[];
} block;

static block_header *free_list = NULL;

void arena_init(void *buf, size_t size) {
    if (size < sizeof(block_header)) {
        free_list = NULL;
        return;
    }
    free_list = (block_header *)buf;
    free_list->size = size - sizeof(block_header);
    free_list->next = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0 || free_list == NULL) return NULL;

    size_t aligned_n = (n + ARENA_ALIGNMENT - 1) & ~(ARENA_ALIGNMENT - 1);

    block_header **prev = &free_list;
    block_header *curr = free_list;

    while (curr) {
        block *block_ptr = (block *)curr;
        if (block_ptr->size >= aligned_n) {
            if (block_ptr->size > aligned_n + sizeof(block_header)) {
                block_header *new_block = (block_header *)((char *)block_ptr + sizeof(block_header) + aligned_n);
                new_block->size = block_ptr->size - aligned_n - sizeof(block_header);
                new_block->next = block_ptr->next;
                curr->next = new_block;
            }
            block_ptr->size = aligned_n;
            *prev = curr->next;
            return block_ptr->data;
        }
        prev = &curr->next;
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;

    block_header **prev = &free_list;
    block_header *curr = free_list;

    char *p_char = (char *)p;
    p_char -= sizeof(block_header);

    block *block_to_free = (block *)p_char;

    while (curr) {
        if (curr == (block_header *)block_to_free) {
            block_to_free->size = block_to_free->size + sizeof(block_header);
            block_to_free->next = free_list;
            free_list = block_to_free;
            return;
        }

        if (curr->next == (block_header *)block_to_free) {
            block_to_free->size += curr->next->size + sizeof(block_header);
            block_to_free->next = curr->next->next;
            return;
        }

        prev = &curr->next;
        curr = curr->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);

    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    size_t aligned_n = (n + ARENA_ALIGNMENT - 1) & ~(ARENA_ALIGNMENT - 1);

    char *p_char = (char *)p;
    p_char -= sizeof(block_header);

    block_header *header = (block_header *)p_char;
    block *block_ptr = (block *)header;

    if (header->size >= aligned_n) {
        return p;
    }

    block_header *next_header = (block_header *)((char *)block_ptr + sizeof(block_header) + block_ptr->size);
    block_header *free_block = next_header->next;

    while (free_block) {
        block *next_free = (block *)free_block;
        if (next_free->size >= aligned_n - header->size) {
            block_header *new_block = (block_header *)((char *)header + sizeof(block_header) + header->size);
            size_t remaining = next_free->size - (aligned_n - header->size);
            if (remaining > sizeof(block_header)) {
                new_block->size = remaining - sizeof(block_header);
                new_block->next = next_free->next;
                free_block->next = new_block;
            } else {
                free_block->next = new_block;
            }
            header->size = aligned_n;
            header->next = free_block;
            return p;
        }
        free_block = free_block->next;
    }

    void *new_p = arena_alloc(n);
    if (new_p) {
        size_t old_size = header->size;
        memcpy(new_p, p, old_size < n ? old_size : n);
        arena_free(p);
    }
    return new_p;
}