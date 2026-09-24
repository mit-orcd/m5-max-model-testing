#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

typedef struct {
    size_t size;
    int free;
    struct block *next;
} block;

static block *head = NULL;
static uint8_t *buffer = NULL;
static size_t bufsize = 0;

static void coalesce(block *b) {
    if (!b) return;
    if (b->next && b->next->free) {
        b->size += sizeof(block) + b->next->size;
        b->next = b->next->next;
    }
    if (b != head) {
        block *prev = head;
        while (prev && prev->next != b) prev = prev->next;
        if (prev && prev->free) {
            prev->size += sizeof(block) + b->size;
            prev->next = b->next;
            b = prev;
        }
    }
}

void arena_init(void *buf, size_t size) {
    buffer = (uint8_t *)buf;
    bufsize = size;
    head = (block *)buffer;
    head->size = bufsize - sizeof(block);
    head->free = 1;
    head->next = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t aligned = (n + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    block *b = head;
    while (b) {
        if (b->free && b->size >= aligned) {
            if (b->size >= aligned + sizeof(block) + _Alignof(max_align_t)) {
                block *newb = (block *)((uint8_t *)b + sizeof(block) + aligned);
                newb->size = b->size - aligned - sizeof(block);
                newb->free = 1;
                newb->next = b->next;
                b->size = aligned;
                b->free = 0;
                b->next = newb;
            } else {
                b->free = 0;
            }
            return (void *)((uint8_t *)b + sizeof(block));
        }
        b = b->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    block *b = (block *)((uint8_t *)p - sizeof(block));
    b->free = 1;
    coalesce(b);
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    size_t aligned = (n + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    block *b = (block *)((uint8_t *)p - sizeof(block));
    if (b->size >= aligned) return p;
    if (b->next && b->next->free) {
        size_t avail = b->size + sizeof(block) + b->next->size;
        if (avail >= aligned) {
            b->size = avail - sizeof(block);
            b->next = b->next->next;
            coalesce(b);
            return p;
        }
    }
    void *newp = arena_alloc(n);
    if (!newp) return NULL;
    memcpy(newp, p, b->size < aligned ? b->size : aligned);
    arena_free(p);
    return newp;
}