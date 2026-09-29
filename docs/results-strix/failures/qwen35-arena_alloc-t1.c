#include <stddef.h>
#include <stdalign.h>
#include <string.h>

typedef struct block_header {
    size_t size;
    int is_free;
    struct block_header *next;
} block_header_t;

#define ALIGNMENT _Alignof(max_align_t)
#define HEADER_SIZE ((sizeof(block_header_t) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))
#define ALIGN_UP(x) (((x) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

static block_header_t *head;
static size_t total_size;

void arena_init(void *buf, size_t size) {
    if (size < HEADER_SIZE + ALIGNMENT) {
        head = NULL;
        total_size = 0;
        return;
    }
    char *aligned_buf = (char *)buf + ALIGNMENT - ((uintptr_t)buf % ALIGNMENT);
    head = (block_header_t *)aligned_buf;
    head->size = size - HEADER_SIZE;
    head->is_free = 1;
    head->next = NULL;
    total_size = size;
}

static block_header_t *find_free_block(size_t n) {
    block_header_t *curr = head;
    while (curr != NULL) {
        if (curr->is_free && curr->size >= n) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

static void merge_with_neighbours(block_header_t *block) {
    if (block->next != NULL && block->next->is_free) {
        block->size += block->next->size + HEADER_SIZE;
        block->next = block->next->next;
    }
    if (head != NULL && head->is_free && head->next == block) {
        head->size += block->size + HEADER_SIZE;
        head->next = block->next;
        block = head;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t req_size = n + HEADER_SIZE;
    req_size = ALIGN_UP(req_size);
    
    block_header_t *block = find_free_block(req_size);
    if (block == NULL) return NULL;

    size_t remaining = block->size - req_size;
    if (remaining > HEADER_SIZE + ALIGNMENT) {
        block_header_t *new_block = (block_header_t *)((char *)block + req_size);
        new_block->size = remaining - HEADER_SIZE;
        new_block->is_free = 1;
        new_block->next = block->next;
        block->next = new_block;
        block->size = req_size;
    }
    
    block->is_free = 0;
    return (char *)block + HEADER_SIZE;
}

void arena_free(void *p) {
    if (p == NULL) return;
    block_header_t *block = (block_header_t *)((char *)p - HEADER_SIZE);
    block->is_free = 1;
    merge_with_neighbours(block);
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (p == NULL) return arena_alloc(n);

    block_header_t *block = (block_header_t *)((char *)p - HEADER_SIZE);
    size_t old_size = block->size;
    size_t req_size = ALIGN_UP(n + HEADER_SIZE);

    if (req_size <= old_size) {
        return p;
    }

    if (block->next != NULL && block->next->is_free) {
        size_t next_size = block->next->size;
        if (block->size + next_size + HEADER_SIZE >= req_size) {
            block->size += next_size + HEADER_SIZE;
            block->next = block->next->next;
            block->is_free = 0;
            return p;
        }
    }

    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    size_t copy_size = (old_size < n) ? old_size : n;
    memcpy(new_p, p, copy_size);
    arena_free(p);
    return new_p;
}