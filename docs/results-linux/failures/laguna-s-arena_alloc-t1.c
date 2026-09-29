#include <stddef.h>
#include <stdalign.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define HDR_SIZE ALIGN(sizeof(struct block))

struct block {
    struct block *next;
    struct block *prev;
    size_t size;
    int free;
};

static struct block *head;

void arena_init(void *buf, size_t size) {
    head = (struct block *)buf;
    head->size = size - HDR_SIZE;
    head->free = 1;
    head->next = NULL;
    head->prev = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    n = ALIGN(n);
    struct block *curr = head;
    while (curr) {
        if (curr->free && curr->size >= n) {
            if (curr->size >= n + HDR_SIZE) {
                struct block *new_block = (struct block *)((char *)curr + HDR_SIZE + n);
                new_block->size = curr->size - n - HDR_SIZE;
                new_block->free = 1;
                new_block->next = curr->next;
                new_block->prev = curr;
                if (curr->next) curr->next->prev = new_block;
                curr->next = new_block;
                curr->size = n;
            }
            curr->free = 0;
            return (char *)curr + HDR_SIZE;
        }
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    struct block *curr = (struct block *)((char *)p - HDR_SIZE);
    curr->free = 1;
    if (curr->prev && curr->prev->free) {
        curr->prev->size += HDR_SIZE + curr->size;
        curr->prev->next = curr->next;
        if (curr->next) curr->next->prev = curr->prev;
    } else if (curr->next && curr->next->free) {
        curr->size += HDR_SIZE + curr->next->size;
        curr->next = curr->next->next;
        if (curr->next) curr->next->prev = curr;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    n = ALIGN(n);
    struct block *curr = (struct block *)((char *)p - HDR_SIZE);
    if (curr->size >= n) {
        if (curr->size >= n + HDR_SIZE && curr->next && curr->next->free) {
            struct block *next = curr->next;
            size_t remaining = curr->size - n;
            if (remaining >= HDR_SIZE) {
                struct block *new_block = (struct block *)((char *)curr + HDR_SIZE + n);
                new_block->size = remaining - HDR_SIZE;
                new_block->free = 1;
                new_block->next = next;
                new_block->prev = curr;
                if (next) {
                    next->prev = new_block;
                    new_block->next = next;
                } else {
                    new_block->next = NULL;
                }
                curr->next = new_block;
                curr->size = n;
                return p;
            }
        }
        return p;
    }
    void *new_p = arena_alloc(n);
    if (!new_p) return NULL;
    size_t copy_size = curr->size < n ? curr->size : n;
    char *dst = (char *)new_p;
    char *src = (char *)p;
    for (size_t i = 0; i < copy_size; i++) dst[i] = src[i];
    arena_free(p);
    return new_p;
}