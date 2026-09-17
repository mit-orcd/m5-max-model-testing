#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    size_t size;
    struct block_header *next;
} block_header;

typedef struct {
    block_header *free_list;
} arena_header;

#define HEADER_SIZE sizeof(arena_header)
#define BLOCK_HEADER_SIZE sizeof(block_header)
#define MAX_ALIGN _Alignof(max_align_t)

static void *align_ptr(void *ptr, size_t align) {
    return (void *)((uintptr_t)ptr + (align - 1) & ~(align - 1));
}

void arena_init(void *buf, size_t size) {
    arena_header *header = buf;
    block_header *initial_block = (block_header *)((uintptr_t)buf + HEADER_SIZE);
    initial_block->size = size - HEADER_SIZE;
    initial_block->next = NULL;
    header->free_list = initial_block;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    arena_header *header = (arena_header *)((uintptr_t)header - HEADER_SIZE);
    block_header *current = header->free_list;
    block_header *prev = NULL;
    while (current) {
        if (current->size >= n) {
            void *ptr = align_ptr(current, BLOCK_HEADER_SIZE);
            ptr = align_ptr(ptr, MAX_ALIGN);
            if (current->size > n) {
                block_header *new_block = (block_header *)((uintptr_t)ptr + n);
                new_block->size = current->size - n;
                new_block->next = current->next;
                if (prev) prev->next = new_block;
                else header->free_list = new_block;
                current->size = n;
            }
            if (prev) prev->next = current->next;
            else if (current == header->free_list) header->free_list = current->next;
            current->next = NULL;
            return ptr;
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    block_header *block = (block_header *)((uintptr_t)p - BLOCK_HEADER_SIZE);
    block->next = NULL;
    arena_header *header = (arena_header *)((uintptr_t)p - HEADER_SIZE - BLOCK_HEADER_SIZE);
    block_header **free_list = &header->free_list;
    while (*free_list) {
        if ((uintptr_t)*free_list > (uintptr_t)block) break;
        if ((uintptr_t)*free_list + (*free_list)->size == (uintptr_t)block) {
            (*free_list)->size += block->size;
            block = *free_list;
            free_list = &block->next;
            continue;
        }
        if ((uintptr_t)block + block->size == (uintptr_t)*free_list) {
            (*free_list)->size += block->size;
            block = *free_list;
            free_list = &block->next;
            continue;
        }
        prev = *free_list;
        free_list = &prev->next;
    }
    block->next = *free_list;
    *free_list = block;
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    block_header *block = (block_header *)((uintptr_t)p - BLOCK_HEADER_SIZE);
    if (block->size >= n) {
        if (block->size > n) {
            block_header *new_block = (block_header *)((uintptr_t)p + n);
            new_block->size = block->size - n;
            new_block->next = block->next;
            block->size = n;
        }
        return p;
    }
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) return NULL;
    memcpy(new_ptr, p, block->size);
    arena_free(p);
    return new_ptr;
}