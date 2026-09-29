#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct Block {
    size_t size;
    struct Block *next;
    int free;
    uint8_t data[];
} Block;

static Block *head = NULL;
static void *buffer = NULL;
static size_t buffer_size = 0;

void arena_init(void *buf, size_t size) {
    buffer = buf;
    buffer_size = size;
    head = (Block *)buf;
    head->size = size - sizeof(Block);
    head->next = NULL;
    head->free = 1;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    Block *prev = NULL;
    for (Block *curr = head; curr != NULL; curr = curr->next) {
        if (curr->free && curr->size >= n) {
            size_t remaining = curr->size - n;
            if (remaining >= sizeof(Block)) {
                Block *new_block = (Block *)((uint8_t *)curr + sizeof(Block) + n);
                new_block->size = remaining - sizeof(Block);
                new_block->next = curr->next;
                new_block->free = 1;
                curr->size = n;
                curr->next = new_block;
            } else {
                curr->size = curr->size;
            }
            curr->free = 0;
            return curr->data;
        }
        prev = curr;
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    Block *block = (Block *)((uint8_t *)p - sizeof(Block));
    block->free = 1;
    Block *curr = head;
    while (curr != NULL && curr != block) {
        curr = curr->next;
    }
    if (curr == NULL) return;
    Block *prev = NULL;
    for (Block *c = head; c != NULL; c = c->next) {
        if (c == block) {
            prev = c;
            break;
        }
        prev = c;
    }
    if (prev != NULL && prev->free) {
        prev->size += sizeof(Block) + block->size;
        prev->next = block->next;
    }
    if (block->next != NULL && block->next->free) {
        block->size += sizeof(Block) + block->next->size;
        block->next = block->next->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    Block *block = (Block *)((uint8_t *)p - sizeof(Block));
    if (block->size >= n) return p;
    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    memcpy(new_p, p, block->size);
    arena_free(p);
    return new_p;
}