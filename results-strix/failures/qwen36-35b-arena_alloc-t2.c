```c
#include <stddef.h>
#include <stdalign.h>

typedef struct Header {
    size_t size;
    int is_free;
    struct Header *next;
} Header;

static Header *g_head = NULL;
static size_t g_total_size = 0;
static char *g_buf_start = NULL;
static char *g_buf_end = NULL;

void arena_init(void *buf, size_t size) {
    g_buf_start = (char *)buf;
    g_buf_end = (char *)buf + size;
    g_total_size = size;
    
    if (size == 0) {
        g_head = NULL;
        return;
    }
    
    // Align the start of the first header
    ptrdiff_t offset = (ptrdiff_t)buf % _Alignof(Header);
    if (offset != 0) {
        offset = _Alignof(Header) - offset;
    }
    
    char *first_header_ptr = (char *)buf + offset;
    g_head = (Header *)first_header_ptr;
    g_head->size = (size_t)(g_buf_end - first_header_ptr) - sizeof(Header);
    g_head->is_free = 1;
    g_head->next = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) {
        return NULL;
    }
    
    // Align the requested size
    size_t aligned_n = n;
    if (aligned_n % _Alignof(Header) != 0) {
        aligned_n = (aligned_n + _Alignof(Header) - 1) & ~(_Alignof(Header) - 1);
    }
    
    // Minimum allocation size to ensure alignment and header overhead
    if (aligned_n < sizeof(Header)) {
        aligned_n = sizeof(Header);
    }
    
    // First-fit search
    Header *current = g_head;
    while (current != NULL) {
        if (current->is_free) {
            // Check if there's enough space for the header + requested data
            // The current block's size field represents the usable space after the header
            if (current->size >= aligned_n) {
                // Found a suitable block
                current->is_free = 0;
                current->size -= aligned_n;
                
                // If there's remaining space, split the block
                if (current->size > 0) {
                    // Create a new header for the remaining space
                    char *next_block_ptr = (char *)current + sizeof(Header) + aligned_n;
                    Header *new_block = (Header *)next_block_ptr;
                    new_block->size = current->size;
                    new_block->is_free = 1;
                    new_block->next = current->next;
                    
                    // Update the next pointer of the new block to match the old next
                    current->next = new_block;
                } else {
                    // No remaining space, the block is exactly the right size
                    current->next = NULL;
                }
                
                // Return pointer to the data area (after the header)
                return (void *)((char *)current + sizeof(Header));
            }
        }
        current = current->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) {
        return;
    }
    
    // Find the header for this block
    char *ptr = (char *)p;
    Header *header = (Header *)(ptr - sizeof(Header));
    
    // Mark as free
    header->is_free = 1;
    
    // Merge with next block if it's free
    if (header->next != NULL && header->next->is_free) {
        // Merge with the next block
        header->size += sizeof(Header) + header->next->size;
        header->next = header->next->next;
    }
    
    // Merge with previous block if it's free
    // We need to traverse from the head to find the previous block
    Header *prev = g_head;
    while (prev != NULL && prev->next != header) {
        prev = prev->next;
    }
    
    if (prev != NULL && prev->is_free) {
        // Merge with the previous block
        prev->size += sizeof(Header) + header->size;
        prev->next = header->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) {
        return arena_alloc(n);
    }
    
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    char *ptr = (char *)p;
    Header *header = (Header *)(ptr - sizeof(Header));
    size_t old_size = header->size;
    
    // Align the requested size
    size_t aligned_n = n;
    if (aligned_n % _Alignof(Header) != 0) {
        aligned_n = (aligned_n + _Alignof(Header) - 1) & ~(_Alignof(Header) - 1);
    }
    
    // Minimum allocation size
    if (aligned_n < sizeof(Header)) {
        aligned_n = sizeof(Header);
    }
    
    // If the new size is less than or equal to the old size, we can keep the block
    // But we need to check if we can merge with the next block if the new size is smaller
    if (aligned_n <= old_size) {
        // We can keep the block
        // If the new size is smaller, we might be able to free some space
        if (aligned_n < old_size) {
            // Decrease the size of the current block
            header->size = old_size - aligned_n;
            
            // If there's remaining space, we need to split the block
            // But the block is currently marked as not free
            // We need to create a new free block from the remaining space
            
            // First, mark the current block as having the new smaller size
            // Then create a new header for the remaining space
            
            // Actually, we need to restructure this. The current block has size old_size.
            // We want to use aligned_n bytes, and leave the rest as free.
            
            // Save the next pointer
            Header *next_block = header->next;
            
            // Update the current block's size to the new allocated size
            header->size = aligned_n;
            
            // Create a new header for the remaining space
            char *remaining_ptr = (char *)header + sizeof(Header) + aligned_n;
            Header *new_free_block = (Header *)remaining_ptr;
            new_free_block->size = header->size + (old_size - aligned_n) - (old_size - aligned_n);
            // Wait, let me recalculate. The old_size was the usable space.
            // Now we want to use aligned_n, so the remaining space is old_size - aligned_n.
            // But we need to account for the new header.
            
            // Let me rethink. The header->size field represents the usable space after the header.
            // So if header->size was old_size, and we want to use aligned_n, then the remaining 
            // usable space is old_size - aligned_n.
            
            // Create a new block for the remaining space
            char *next_data_ptr = (char *)header + sizeof(Header) + aligned_n;
            Header *new_block = (Header *)next_data_ptr;
            
            // The new block's size is the remaining usable space
            size_t remaining_usable = old_size - aligned_n;
            if (remaining_usable > 0) {
                new_block->size = remaining_usable;
                new_block->is_free = 1;
                new_block->next = next_block;
                header->next = new_block;
            } else {
                header->next = next_block;
            }
            
            // Now check if we can