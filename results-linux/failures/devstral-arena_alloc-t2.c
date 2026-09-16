#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGNMENT _Alignof(max_align_t)
#define MIN_BLOCK_SIZE (sizeof(struct block) + ALIGNMENT - 1)
#define BLOCK_HEADER_SIZE (sizeof(struct block))

struct block {
    struct block *next;
    size_t size;
    uint32_t is_free : 1;
    uint32_t : 31;
    uint8_t data[];
};

static struct block *head = NULL;
static void *buf = NULL;
static size_t buf_size = 0;

void arena_init(void *buf, size_t size) {
    if (buf == NULL || size == 0) return;
    this->buf = buf;
    this->buf_size = size;
    head = (struct block *)buf;
    head->next = NULL;
    head->size = size - BLOCK_HEADER_SIZE;
    head->is_free = 1;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    if (head == NULL) return NULL;

    struct block *prev = NULL;
    struct block *current = head;
    size_t required_size = n + BLOCK_HEADER_SIZE;

    while (current != NULL) {
        if (current->is_free && current->size >= n) {
            if (current->size >= required_size) {
                struct block *new_block = (struct block *)((uint8_t *)current + BLOCK_HEADER_SIZE + n);
                new_block->next = current->next;
                new_block->size = current->size - (n + BLOCK_HEADER_SIZE);
                new_block->is_free = 1;
                current->next = new_block;
                current->size = n;
                current->is_free = 0;
                return current->data;
            } else {
                current->is_free = 0;
                return current->data;
            }
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    struct block *block = (struct block *)((uint8_t *)p - BLOCK_HEADER_SIZE);
    if (block->is_free) return;
    block->is_free = 1;

    struct block *current = head;
    struct block *prev = NULL;
    while (current != NULL && current != block) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) return;

    if (prev != NULL && prev->is_free) {
        prev->size += BLOCK_HEADER_SIZE + current->size;
        prev->next = current->next;
        block = prev;
    }

    if (current->next != NULL && current->next->is_free) {
        block->size += BLOCK_HEADER_SIZE + current->next->size;
        block->next = current->next->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    struct block *block = (struct block *)((uint8_t *)p - BLOCK_HEADER_SIZE);
    if (!block->is_free && block->size >= n) {
        if (block->next != NULL && block->next->is_free && (block->size + BLOCK_HEADER_SIZE + block->next->size) >= n) {
            block->size += BLOCK_HEADER_SIZE + block->next->size;
            block->next = block->next->next;
            return p;
        }
        return p;
    }

    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;

    size_t copy_size = (n < block->size) ? n : block->size;
    memcpy(new_p, p, copy_size);
    arena_free(p);
    return new_p;
}