#include <stddef.h>
#include <stddef.h>
#include <string.h>

typedef struct block_header {
    size_t size;
    int is_free;
    struct block_header *next;
    struct block_header *prev;
} block_header_t;

static struct {
    void *buffer;
    size_t size;
    block_header_t *head;
} g_arena;

static size_t align_up(size_t size, size_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

void arena_init(void *buf, size_t size) {
    g_arena.buffer = buf;
    g_arena.size = size;
    
    // Ensure the buffer can hold at least the header
    if (size < sizeof(block_header_t)) {
        g_arena.head = NULL;
        return;
    }

    g_arena.head = (block_header_t *)buf;
    g_arena.head->size = size;
    g_arena.head->is_free = 1;
    g_arena.head->next = NULL;
    g_arena.head->prev = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;

    size_t needed_size = align_up(n, _Alignof(max_align_t));
    size_t header_size = sizeof(block_header_t);
    
    // Total size required for a block is header + payload
    // But the header itself must be aligned.
    // We store the payload size in 'size' and the header is part of the block.
    // To simplify: 'size' is the total size of the block including header.
    
    block_header_t *curr = g_arena.head;
    while (curr) {
        if (curr->is_free && curr->size >= (n + header_size)) {
            // Split block if there is enough space for a new header + minimum payload
            // We need enough space to hold a header and at least 1 byte of payload
            size_t total_needed = n + header_size;
            if (curr->size >= total_needed + header_size + 1) {
                block_header_t *next_block = (block_header_t *)((char *)curr + total_needed);
                next_block->size = curr->size - total_needed;
                next_block->is_free = 1;
                next_block->next = curr->next;
                next_block->prev = curr;
                if (curr->next) curr->next->prev = next_block;
                curr->next = next_block;
                curr->size = total_needed;
            }
            
            curr->is_free = 0;
            return (void *)((char *)curr + header_size);
        }
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;

    block_header_t *curr = (block_header_t *)((char *)p - sizeof(block_header_t));
    curr->is_free = 1;

    // Merge with next
    if (curr->next && curr->next->is_free) {
        curr->size += curr->next->size;
        curr->next = curr->next->next;
        if (curr->next) curr->next->prev = curr;
    }

    // Merge with prev
    if (curr->prev && curr->prev->is_free) {
        curr->prev->size += curr->size;
        curr->prev->next = curr->next;
        if (curr->next) curr->next->prev = curr->prev;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    block_header_t *curr = (block_header_t *)((char *)p - sizeof(block_header_t));
    size_t old_payload_size = curr->size - sizeof(block_header_t);

    if (n <= old_payload_size) {
        return p;
    }

    // Try to grow into next block
    if (curr->next && curr->next->is_free) {
        size_t combined_size = curr->size + curr->next->size;
        if (combined_size >= n + sizeof(block_header_t)) {
            // Merge with next
            block_header_t *next_block = curr->next;
            curr->size = combined_size;
            curr->next = next_block->next;
            if (curr->next) curr->next->prev = curr;
            
            // If we still have excess space, split it
            size_t needed_total = n + sizeof(block_header_t);
            if (curr->size > needed_total + sizeof(block_header_t) + 1) {
                block_header_t *split = (block_header_t *)((char *)curr + needed_total);
                split->size = curr->size - needed_total;
                split->is_free = 1;
                split->next = curr->next;
                split->prev = curr;
                if (curr->next) curr->next->prev = split;
                curr->next = split;
                curr->size = needed_total;
            }
            return p;
        }
    }

    // Otherwise, allocate new and move
    void *new_p = arena_alloc(n);
    if (new_p) {
        size_t copy_size = (old_payload_size < n) ? old_payload_size : n;
        memcpy(new_p, p, copy_size);
        arena_free(p);
    }
    return new_p;
}