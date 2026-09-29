#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct block {
    size_t size;
    struct block *next;
    int free;
} block;

static block *head = NULL;
static void *buf_start = NULL;
static size_t buf_size = 0;

void arena_init(void *buf, size_t size) {
    if (buf == NULL || size < sizeof(block)) return;
    buf_start = buf;
    buf_size = size;
    head = (block *)buf;
    head->size = size - sizeof(block);
    head->next = NULL;
    head->free = 1;
}

void *arena_alloc(size_t n) {
    if (n == 0 || buf_start == NULL) return NULL;
    block *current = head;
    block *prev = NULL;
    while (current != NULL) {
        if (current->free && current->size >= n) {
            size_t remaining = current->size - n;
            if (remaining >= sizeof(block)) {
                block *new = (block *)((uint8_t *)current + sizeof(block) + n);
                new->size = remaining - sizeof(block);
                new->next = current->next;
                new->free = 1;
                current->size = n;
                current->next = new;
            }
            current->free = 0;
            return (uint8_t *)current + sizeof(block);
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    block *current = (block *)((uint8_t *)p - sizeof(block));
    if (current->free) return;
    current->free = 1;
    block *next = current->next;
    if (next != NULL && next->free) {
        current->size += sizeof(block) + next->size;
        current->next = next->next;
    }
    block *prev = head;
    while (prev != NULL && prev->next != current) {
        prev = prev->next;
    }
    if (prev != NULL && prev->free) {
        prev->size += sizeof(block) + current->size;
        prev->next = current->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    block *current = (block *)((uint8_t *)p - sizeof(block));
    if (!current->free && current->size >= n) {
        return p;
    }
    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    size_t copy_size = (n < current->size) ? n : current->size;
    memcpy(new_p, p, copy_size);
    arena_free(p);
    return new_p;
}