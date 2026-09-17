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
    int free;
};

static struct block_header *head = NULL;
static void *buffer_start = NULL;
static size_t buffer_size = 0;

void arena_init(void *buf, size_t size) {
    if (size < MIN_BLOCK_SIZE) return;
    buffer_start = buf;
    buffer_size = size;
    head = (struct block_header *)buf;
    head->size = size - BLOCK_HEADER_SIZE;
    head->next = NULL;
    head->free = 1;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    n = ALIGN_UP(n);
    struct block_header *current = head;
    struct block_header *prev = NULL;
    struct block_header *best_fit = NULL;
    size_t best_size = SIZE_MAX;

    while (current != NULL) {
        if (current->free && current->size >= n) {
            if (current->size < best_size) {
                best_fit = current;
                best_size = current->size;
            }
        }
        prev = current;
        current = current->next;
    }

    if (best_fit == NULL) return NULL;

    if (best_fit->size >= n + BLOCK_HEADER_SIZE + ALIGNMENT) {
        struct block_header *new_block = (struct block_header *)((char *)best_fit + BLOCK_HEADER_SIZE + n);
        new_block->size = best_fit->size - n - BLOCK_HEADER_SIZE;
        new_block->next = best_fit->next;
        new_block->free = 1;
        best_fit->size = n;
        best_fit->next = new_block;
    }

    best_fit->free = 0;
    return (char *)best_fit + BLOCK_HEADER_SIZE;
}

void arena_free(void *p) {
    if (p == NULL) return;
    struct block_header *block = (struct block_header *)((char *)p - BLOCK_HEADER_SIZE);
    block->free = 1;

    struct block_header *current = head;
    struct block_header *prev = NULL;

    while (current != NULL && current != block) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) return;

    struct block_header *next_block = current->next;
    if (next_block != NULL && next_block->free) {
        current->size += next_block->size + BLOCK_HEADER_SIZE;
        current->next = next_block->next;
    }

    if (prev != NULL && prev->free) {
        prev->size += current->size + BLOCK_HEADER_SIZE;
        prev->next = current->next;
    } else if (current == head) {
        head = current;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    n = ALIGN_UP(n);
    struct block_header *block = (struct block_header *)((char *)p - BLOCK_HEADER_SIZE);
    size_t old_size = block->size;

    if (block->free) return NULL;

    if (old_size >= n) return p;

    struct block_header *next_block = block->next;
    if (next_block != NULL && next_block->free && block->size + next_block->size + BLOCK_HEADER_SIZE >= n) {
        block->size += next_block->size + BLOCK_HEADER_SIZE;
        block->next = next_block->next;
        return p;
    }

    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    size_t copy_size = old_size < n ? old_size : n;
    memcpy(new_p, p, copy_size);
    arena_free(p);
    return new_p;
}