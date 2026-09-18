#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define HEADER_SIZE ALIGN_UP(sizeof(struct block_header))

static unsigned char *arena_base = NULL;
static size_t arena_size = 0;

struct block_header {
    size_t size;
    int free;
    struct block_header *next;
    struct block_header *prev;
};

static struct block_header *head = NULL;

void arena_init(void *buf, size_t size) {
    arena_base = (unsigned char *)buf;
    arena_size = size;
    head = NULL;
}

static struct block_header *find_free_block(size_t n) {
    struct block_header *cur = head;
    while (cur) {
        if (cur->free && cur->size >= n)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

static struct block_header *split_block(struct block_header *b, size_t n) {
    if (b->size > n + sizeof(struct block_header)) {
        struct block_header *new_block = (struct block_header *)((unsigned char *)b + HEADER_SIZE + n);
        new_block->size = b->size - n - HEADER_SIZE;
        new_block->free = 1;
        new_block->next = b->next;
        new_block->prev = b;
        if (b->next)
            b->next->prev = new_block;
        b->size = n;
        b->next = new_block;
    }
    return b;
}

static struct block_header *merge_blocks(struct block_header *b) {
    struct block_header *cur = head;
    while (cur) {
        if (cur->free && cur->next && cur->next->free) {
            cur->size += HEADER_SIZE + cur->next->size;
            cur->next = cur->next->next;
            if (cur->next)
                cur->next->prev = cur;
        } else {
            cur = cur->next;
        }
    }
    return b;
}

void *arena_alloc(size_t n) {
    if (n == 0)
        return NULL;

    size_t alloc_size = ALIGN_UP(n);
    if (head == NULL) {
        if (alloc_size + HEADER_SIZE > arena_size)
            return NULL;
        head = (struct block_header *)arena_base;
        head->size = alloc_size;
        head->free = 0;
        head->next = NULL;
        head->prev = NULL;
        return (void *)((unsigned char *)head + HEADER_SIZE);
    }

    struct block_header *block = find_free_block(alloc_size);
    if (!block)
        return NULL;

    block = split_block(block, alloc_size);
    block->free = 0;
    return (void *)((unsigned char *)block + HEADER_SIZE);
}

void arena_free(void *p) {
    if (!p)
        return;

    struct block_header *block = (struct block_header *)((unsigned char *)p - HEADER_SIZE);
    block->free = 1;
    merge_blocks(block);
}

void *arena_realloc(void *p, size_t n) {
    if (!p)
        return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    struct block_header *block = (struct block_header *)((unsigned char *)p - HEADER_SIZE);
    size_t new_size = ALIGN_UP(n);

    if (block->free == 0 && block->size >= new_size)
        return p;

    if (block->next && block->next->free && block->size + HEADER_SIZE + block->next->size >= new_size) {
        block->size += HEADER_SIZE + block->next->size;
        block->next = block->next->next;
        if (block->next)
            block->next->prev = block;
        return p;
    }

    void *newp = arena_alloc(n);
    if (!newp)
        return NULL;
    memcpy(newp, p, block->size < new_size ? block->size : new_size);
    arena_free(p);
    return newp;
}