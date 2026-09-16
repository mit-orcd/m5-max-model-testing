#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct Block {
    size_t size;
    struct Block *next;
    int free;
} Block;

static void *arena_buffer = NULL;
static size_t arena_total_size = 0;
static Block *arena_head = NULL;

void arena_init(void *buf, size_t size) {
    arena_buffer = buf;
    arena_total_size = size;
    arena_head = (Block *)buf;
    arena_head->size = size - sizeof(Block);
    arena_head->next = NULL;
    arena_head->free = 1;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    const size_t alignment = _Alignof(max_align_t);
    size_t aligned_n = (n + alignment - 1) & -(alignment);
    size_t required_space = aligned_n + sizeof(Block);
    
    Block *current = arena_head;
    Block *prev = NULL;
    
    while (current != NULL) {
        if (current->free && current->size >= required_space) {
            if (current->size >= required_space + sizeof(Block) + alignment) {
                Block *new_block = (Block *)((char *)current + sizeof(Block) + aligned_n);
                new_block->size = current->size - aligned_n - sizeof(Block);
                new_block->next = current->next;
                new_block->free = 1;
                current->size = aligned_n;
                current->next = new_block;
                current->free = 0;
            } else {
                current->free = 0;
            }
            return (char *)current + sizeof(Block);
        }
        prev = current;
        current = current->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    
    Block *block = (Block *)((char *)p - sizeof(Block));
    block->free = 1;
    
    Block *current = arena_head;
    while (current != NULL) {
        if (current->free) {
            Block *next = current->next;
            char *end_current = (char *)current + sizeof(Block) + current->size;
            if (next != NULL && next->free) {
                current->size += sizeof(Block) + next->size;
                current->next = next->next;
            }
            char *prev_end = (char *)current + sizeof(Block) + current->size;
            if (current != arena_head) {
                Block *prev = arena_head;
                while (prev->next != current && prev->next != NULL) {
                    prev = prev->next;
                }
                if (prev->free && (char *)prev + sizeof(Block) + prev->size == (char *)current) {
                    prev->size += sizeof(Block) + current->size;
                    prev->next = current->next;
                }
            }
        }
        current = current->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    Block *block = (Block *)((char *)p - sizeof(Block));
    size_t old_size = block->size;
    
    const size_t alignment = _Alignof(max_align_t);
    size_t aligned_n = (n + alignment - 1) & -(alignment);
    
    if (aligned_n <= old_size) {
        return p;
    }
    
    Block *next_block = block->next;
    if (next_block != NULL && next_block->free) {
        size_t combined_size = old_size + sizeof(Block) + next_block->size;
        if (combined_size >= aligned_n) {
            size_t needed = aligned_n - old_size;
            if (needed <= next_block->size) {
                block->size = aligned_n;
                Block *new_block = (Block *)((char *)block + sizeof(Block) + aligned_n);
                new_block->size = next_block->size - needed;
                new_block->next = next_block->next;
                new_block->free = 1;
                block->next = new_block;
                return p;
            }
        }
    }
    
    void *new_ptr = arena_alloc(n);
    if (new_ptr != NULL) {
        size_t copy_size = old_size < n ? old_size : n;
        memcpy(new_ptr, p, copy_size);
        arena_free(p);
    }
    return new_ptr;
}