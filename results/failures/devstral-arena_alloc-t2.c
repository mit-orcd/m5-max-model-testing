#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct __attribute__((__aligned__(8))) Block {
    Block *prev;
    Block *next;
    size_t size;
    uint8_t used;
} Block;

static Block *free_head = NULL;
static Block *alloc_head = NULL;
static Block *arena_start = NULL;
static Block *arena_end = NULL;

void arena_init(void *buf, size_t size) {
    if (size < sizeof(Block)) return;

    uint8_t *mem = (uint8_t *)buf;
    arena_start = (Block *)mem;
    arena_end = (Block *)mem + size;

    free_head = arena_start;
    alloc_head = (Block *)((uint8_t *)arena_start + sizeof(Block));
    free_head->prev = NULL;
    free_head->next = alloc_head;
    free_head->size = size - sizeof(Block);
    free_head->used = 0;

    alloc_head->prev = free_head;
    alloc_head->next = NULL;
    alloc_head->size = 0;
    alloc_head->used = 1;
}

void *arena_alloc(size_t n) {
    if (n == 0 || n > (size_t)(arena_end - arena_start)) return NULL;

    Block *prev = NULL;
    Block *curr = free_head;
    while (curr != NULL) {
        if (curr->size >= n) {
            if (curr->size > n + sizeof(Block)) {
                Block *new_block = (Block *)((uint8_t *)curr + sizeof(Block) + n);
                new_block->size = curr->size - n - sizeof(Block);
                new_block->used = 0;
                new_block->prev = curr;
                new_block->next = curr->next;
                if (curr->next != NULL) curr->next->prev = new_block;
                curr->next = new_block;
                curr->size = n;
            }
            curr->used = 1;
            if (prev == NULL) {
                free_head = curr->next;
                if (free_head != NULL) free_head->prev = NULL;
            } else {
                prev->next = curr->next;
                if (curr->next != NULL) curr->next->prev = prev;
            }
            return (uint8_t *)curr + sizeof(Block);
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;

    Block *block = (Block *)((uint8_t *)p - sizeof(Block));
    block->used = 0;

    if (free_head == NULL) {
        free_head = block;
        block->prev = NULL;
        block->next = NULL;
        return;
    }

    Block *curr = free_head;
    while (curr != NULL && curr < block) {
        curr = curr->next;
    }

    if (curr == NULL) {
        free_head = block;
        block->prev = NULL;
        block->next = NULL;
        return;
    }

    block->next = curr;
    block->prev = curr->prev;
    if (curr->prev != NULL) curr->prev->next = block;
    else free_head = block;
    curr->prev = block;

    Block *prev = block->prev;
    Block *next = block->next;

    if (prev != NULL && prev->used == 0) {
        prev->size += sizeof(Block) + block->size;
        prev->next = next;
        if (next != NULL) next->prev = prev;
        block = prev;
    }

    if (next != NULL && next->used == 0) {
        block->size += sizeof(Block) + next->size;
        block->next = next->next;
        if (next->next != NULL) next->next->prev = block;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    Block *block = (Block *)((uint8_t *)p - sizeof(Block));
    size_t old_size = block->size;

    if (n > old_size) {
        Block *next = block->next;
        if (next != NULL && next->used == 0 && next->size >= n - old_size) {
            block->size = n;
            block->next = next->next;
            if (next->next != NULL) next->next->prev = block;
            return p;
        }
    }

    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;

    memcpy(new_p, p, old_size);
    arena_free(p);
    return new_p;
}