#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct block {
    size_t size;
    struct block *next;
} block_t;

static void *arena_buffer = NULL;
static size_t arena_size = 0;
static block_t *free_head = NULL;

void arena_init(void *buf, size_t size) {
    arena_buffer = buf;
    arena_size = size;
    free_head = NULL;
    if (size > 0) {
        block_t *initial = (block_t *)buf;
        initial->size = size - sizeof(block_t);
        initial->next = NULL;
        free_head = initial;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    if (n > arena_size) return NULL;

    block_t *prev = NULL;
    block_t *curr = free_head;

    while (curr != NULL) {
        size_t required = n + sizeof(block_t);
        if (curr->size >= required) {
            if (prev == NULL) {
                free_head = curr->next;
            } else {
                prev->next = curr->next;
            }
            if (curr->size > required) {
                block_t *new_block = (block_t *)((char *)curr + required);
                new_block->size = curr->size - required;
                new_block->next = curr->next;
                curr->next = new_block;
            }
            return (char *)curr + sizeof(block_t);
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;

    block_t *to_free = (block_t *)((char *)p - sizeof(block_t));

    if (free_head == NULL) {
        to_free->next = NULL;
        free_head = to_free;
        return;
    }

    block_t *curr = free_head;
    block_t *prev = NULL;

    while (curr != NULL && curr < to_free) {
        prev = curr;
        curr = curr->next;
    }

    if (prev == NULL) {
        free_head = to_free;
    } else {
        prev->next = to_free;
    }

    to_free->next = curr;

    if (curr != NULL && (char *)curr == (char *)to_free + sizeof(block_t) + to_free->size) {
        to_free->size += sizeof(block_t) + curr->size;
        to_free->next = curr->next;
    }

    if (prev != NULL && (char *)prev + sizeof(block_t) + prev->size == (char *)to_free) {
        prev->size += sizeof(block_t) + to_free->size;
        prev->next = to_free->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    block_t *curr_block = (block_t *)((char *)p - sizeof(block_t));
    size_t old_size = curr_block->size;
    size_t new_size = n;

    if (new_size <= old_size) {
        return p;
    }

    block_t *next_block = curr_block->next;
    if (next_block != NULL && (char *)next_block == (char *)curr_block + sizeof(block_t) + curr_block->size) {
        size_t total_available = old_size + sizeof(block_t) + next_block->size;
        if (new_size <= total_available) {
            curr_block->size = new_size;
            curr_block->next = next_block->next;
            return p;
        }
    }

    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;

    size_t copy_size = old_size < new_size ? old_size : new_size;
    memcpy(new_p, p, copy_size);
    arena_free(p);
    return new_p;
}