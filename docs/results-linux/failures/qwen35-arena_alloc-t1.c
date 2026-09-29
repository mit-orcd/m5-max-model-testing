#include <stddef.h>
#include <stdalign.h>
#include <string.h>
#include <stdint.h>

#define MAX_ALIGN _Alignof(max_align_t)
#define BLOCK_HEADER_SIZE ((sizeof(size_t) + MAX_ALIGN - 1) / MAX_ALIGN * MAX_ALIGN)

typedef struct block_header {
    size_t size;
    int is_free;
    struct block_header* next;
    struct block_header* prev;
} block_header_t;

static block_header_t* head = NULL;
static size_t total_size = 0;

void arena_init(void *buf, size_t size) {
    if (size < sizeof(block_header_t) || (uintptr_t)buf % MAX_ALIGN != 0) {
        head = NULL;
        total_size = 0;
        return;
    }
    
    // Align the buffer start to MAX_ALIGN if necessary, but since buf is passed,
    // we assume it is aligned. We adjust size to account for header alignment.
    // The first block starts immediately after the user buffer pointer.
    // We need to ensure the header itself is aligned.
    uintptr_t start = (uintptr_t)buf;
    
    // The allocator will manage the buffer starting from 'buf'.
    // We construct a free block that spans the entire buffer.
    // The header must be aligned.
    uintptr_t header_ptr = start;
    // If the start isn't aligned, we can't use it as a header. 
    // However, the spec says arena_init hands the allocator a buffer. 
    // We assume the buffer passed is aligned. If not, we might need to skip bytes.
    // For simplicity and strict compliance with "aligned for any type", 
    // we assume the buffer pointer is aligned. If not, we shift.
    
    if (header_ptr % MAX_ALIGN != 0) {
        size_t padding = MAX_ALIGN - (header_ptr % MAX_ALIGN);
        header_ptr += padding;
        size -= padding;
    }

    if (size < sizeof(block_header_t)) {
        head = NULL;
        total_size = 0;
        return;
    }

    block_header_t* first = (block_header_t*)header_ptr;
    first->size = size - sizeof(block_header_t);
    first->is_free = 1;
    first->next = NULL;
    first->prev = NULL;
    
    head = first;
    total_size = size;
}

static void merge(block_header_t* block) {
    if (!block) return;

    // Merge with next
    if (block->next && block->next->is_free) {
        block->size += sizeof(block_header_t) + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }

    // Merge with prev
    if (block->prev && block->prev->is_free) {
        block->prev->size += sizeof(block_header_t) + block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
    }
}

void* arena_alloc(size_t n) {
    if (n == 0 || head == NULL) return NULL;

    size_t aligned_n = (n + MAX_ALIGN - 1) / MAX_ALIGN * MAX_ALIGN;
    // We need space for data + header
    size_t required = aligned_n + sizeof(block_header_t);
    
    block_header_t* current = head;
    
    while (current) {
        if (current->is_free && current->size >= aligned_n) {
            // Found a suitable block
            if (current->size >= required + sizeof(block_header_t)) {
                // Split the block
                block_header_t* new_block = (block_header_t*)((char*)current + sizeof(block_header_t) + aligned_n);
                new_block->size = current->size - aligned_n - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = current->next;
                new_block->prev = current->prev;
                if (current->next) {
                    current->next->prev = new_block;
                }
                current->next = new_block;
                current->size = aligned_n;
                current->is_free = 0;
                return (char*)current + sizeof(block_header_t);
            } else {
                // Use the whole block
                current->is_free = 0;
                return (char*)current + sizeof(block_header_t);
            }
        }
        current = current->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;

    block_header_t* block = (block_header_t*)((char*)p - sizeof(block_header_t));
    
    // Basic validation (optional, but good for safety)
    if (block < head) return;
    
    block->is_free = 1;
    merge(block);
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    block_header_t* block = (block_header_t*)((char*)p - sizeof(block_header_t));
    
    // Calculate required size
    size_t aligned_n = (n + MAX_ALIGN - 1) / MAX_ALIGN * MAX_ALIGN;
    size_t required = aligned_n + sizeof(block_header_t);
    size_t current_data_size = block->size;

    if (block->size >= aligned_n) {
        // Block is large enough, just update metadata if needed (or keep as is)
        // The spec says: "Contents up to the smaller of the old and new sizes are preserved."
        // If we don't move, we just change the size field? 
        // Actually, if we don't move, the internal fragmentation changes.
        // But if n is smaller, we just treat it as allocated with new size?
        // Standard realloc behavior: if n < old, we just keep the block, but the next alloc 
        // might see a smaller free block if we split.
        // However, if n is larger, we might need to grow.
        
        // If we just need more space and the next block is free and large enough, grow into it.
        if (block->next && block->next->is_free) {
            size_t next_data_size = block->next->size;
            size_t total_free = block->size + sizeof(block_header_t) + next_data_size;
            
            if (total_free >= required) {
                // Grow into next block
                // We need to merge the next block into this one, but we must preserve the content
                // The next block is free, so no content to preserve there.
                block->size = total_free - sizeof(block_header_t);
                block->next = block->next->next;
                if (block->next) {
                    block->next->prev = block;
                }
                return p;
            }
        }
        
        // If we don't grow into next, we just keep the block.
        // If n is smaller than current allocation, we effectively shrink the usable size 
        // but don't free the rest unless we split. 
        // To be safe and consistent with "realloc", if we don't move, we just return p.
        // The internal size of the block remains the same? 
        // Actually, if we request smaller, we can just keep the block as is, 
        // the user won't access the extra space.
        // But if we request larger, and can't grow, we must fail or move.
        // The prompt says "realloc behaves like realloc".
        // If we can't satisfy the request without moving, we should try to move?
        // But moving requires a new allocation.
        // The prompt implies a fixed buffer, so we cannot move if no space exists.
        // So if we can't grow in place, we return NULL?
        // Standard realloc returns NULL and leaves original if fails.
        
        if (block->size < aligned_n) {
            return NULL;
        }
        return p;
    } else {
        // Current block is too small, need to move or fail.
        // Try to find a new block
        void* new_p = arena_alloc(n);
        if (new_p == NULL) {
            return NULL;
        }
        memcpy(new_p, p, block->size);
        arena_free(p);
        return new_p;
    }
}