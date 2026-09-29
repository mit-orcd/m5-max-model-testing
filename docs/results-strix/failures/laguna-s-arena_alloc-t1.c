#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

#define ALIGN_UP(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct Block {
    size_t size;
    struct Block *next;
    struct Block *prev;
    int free;
} Block;

static Block *free_list = NULL;
static Block *used_list = NULL;
static size_t total_size;
static void *buffer_start;

void arena_init(void *buf, size_t size) {
    buffer_start = buf;
    total_size = size;
    free_list = (Block *)buf;
    free_list->size = size - sizeof(Block);
    free_list->next = NULL;
    free_list->prev = NULL;
    free_list->free = 1;
    used_list = NULL;
}

static void remove_from_free_list(Block *block) {
    if (block->prev) block->prev->next = block->next;
    else free_list = block->next;
    if (block->next) block->next->prev = block->prev;
    block->prev = NULL;
    block->next = NULL;
}

static void add_to_used_list(Block *block) {
    block->next = used_list;
    block->prev = NULL;
    if (used_list) used_list->prev = block;
    used_list = block;
}

static void remove_from_used_list(Block *block) {
    if (block->prev) block->prev->next = block->next;
    else used_list = block->next;
    if (block->next) block->next->prev = block->prev;
    block->prev = NULL;
    block->next = NULL;
}

static void add_to_free_list(Block *block) {
    block->next = free_list;
    block->prev = NULL;
    if (free_list) free_list->prev = block;
    free_list = block;
}

static void merge_blocks(Block *block) {
    Block *next_block = (Block *)((char *)block + sizeof(Block) + block->size);
    if ((char *)next_block < (char *)buffer_start + total_size && next_block->free) {
        remove_from_free_list(next_block);
        block->size += sizeof(Block) + next_block->size;
    }
    if (block->prev && ((Block *)((char *)block - sizeof(Block)))->free &&
        (char *)block - sizeof(Block) >= (char *)buffer_start) {
        Block *prev_block = block->prev;
        remove_from_free_list(prev_block);
        prev_block->size += sizeof(Block) + block->size;
        block = prev_block;
        remove_from_used_list(block);
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t aligned_n = ALIGN_UP(n, alignof(max_align_t));
    Block *current = free_list;
    while (current) {
        if (current->free && current->size >= aligned_n) {
            remove_from_free_list(current);
            if (current->size > aligned_n + sizeof(Block)) {
                Block *new_block = (Block *)((char *)current + sizeof(Block) + aligned_n);
                new_block->size = current->size - sizeof(Block) - aligned_n;
                new_block->free = 1;
                new_block->next = NULL;
                new_block->prev = NULL;
                add_to_free_list(new_block);
                current->size = aligned_n;
            }
            current->free = 0;
            add_to_used_list(current);
            return (char *)current + sizeof(Block);
        }
        current = current->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    Block *block = (Block *)((char *)p - sizeof(Block));
    remove_from_used_list(block);
    block->free = 1;
    add_to_free_list(block);
    merge_blocks(block);
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    size_t aligned_n = ALIGN_UP(n, alignof(max_align_t));
    Block *block = (Block *)((char *)p - sizeof(Block));
    if (aligned_n <= block->size) return p;
    Block *next_block = (Block *)((char *)block + sizeof(Block) + block->size);
    if ((char *)next_block < (char *)buffer_start + total_size && next_block->free &&
        next_block->size >= aligned_n - block->size) {
        remove_from_free_list(next_block);
        if (next_block->size > aligned_n - block->size + sizeof(Block)) {
            Block *new_block = (Block *)((char *)block + sizeof(Block) + aligned_n);
            new_block->size = next_block->size - (aligned_n - block->size) - sizeof(Block);
            new_block->free = 1;
            new_block->next = NULL;
            new_block->prev = NULL;
            add_to_free_list(new_block);
            next_block->size = aligned_n - block->size;
        }
        block->size = aligned_n;
        return p;
    }
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) return NULL;
    size_t copy_size = MIN(block->size, aligned_n);
    char *src = (char *)p;
    char *dst = (char *)new_ptr;
    for (size_t i = 0; i < copy_size; i++) dst[i] = src[i];
    arena_free(p);
    return new_ptr;
}