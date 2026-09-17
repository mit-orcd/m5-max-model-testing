```c
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

typedef struct {
    uint8_t *start;
    uint8_t *end;
} Arena;

typedef struct {
    size_t size;
    bool free;
} Header;

static Arena arena;

#define ALIGN_UP(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

static uint8_t *align_up(uint8_t *ptr) {
    uintptr_t addr = (uintptr_t)ptr;
    uintptr_t aligned = ALIGN_UP(addr, _Alignof(max_align_t));
    return (uint8_t *)aligned;
}

static uint8_t *align_down(uint8_t *ptr) {
    uintptr_t addr = (uintptr_t)ptr;
    uintptr_t aligned = addr & ~((uintptr_t)_Alignof(max_align_t) - 1);
    return (uint8_t *)aligned;
}

void arena_init(void *buf, size_t size) {
    arena.start = (uint8_t *)buf;
    arena.end = arena.start + size;
}

static uint8_t *find_free_block(size_t total_needed) {
    uint8_t *ptr = arena.start;
    while (ptr < arena.end) {
        Header *hdr = (Header *)ptr;
        if (hdr->free) {
            size_t hdr_size = sizeof(Header);
            size_t data_start = (size_t)(ptr + hdr_size);
            size_t data_end = (size_t)(ptr + hdr_size + hdr->size);
            size_t block_end = data_end;
            
            // Check if the block after this one is also free (shouldn't happen if we merge, but just in case)
            // Actually, we merge on free, so free blocks should be maximal.
            // But we need to check if this free block is large enough.
            
            // The total size available in this block is hdr->size (data size)
            // We need: align_up(ptr + hdr_size) to align_up(ptr + hdr_size + hdr->size) to have at least total_needed bytes?
            // No, we just need the data area to be at least total_needed bytes after alignment.
            
            uint8_t *data_ptr = align_up(ptr + hdr_size);
            size_t available = (size_t)(ptr + hdr_size + hdr->size - data_ptr);
            
            if (available >= total_needed) {
                return ptr;
            }
        }
        // Move to next block
        if (hdr->free) {
            ptr += sizeof(Header) + hdr->size;
        } else {
            ptr += sizeof(Header) + hdr->size;
        }
    }
    return NULL;
}

static void mark_free(Header *hdr) {
    hdr->free = true;
}

static void mark_used(Header *hdr) {
    hdr->free = false;
}

static void merge_with_next(Header *hdr) {
    // Check if next block exists and is free
    uint8_t *next_ptr = (uint8_t *)hdr + sizeof(Header) + hdr->size;
    if (next_ptr < arena.end) {
        Header *next_hdr = (Header *)next_ptr;
        if (next_hdr->free) {
            // Merge
            hdr->size += sizeof(Header) + next_hdr->size;
            // The next block is now absorbed
        }
    }
}

static void merge_with_prev(Header *hdr) {
    // Check if previous block exists and is free
    uint8_t *prev_ptr = (uint8_t *)hdr - sizeof(Header);
    if (prev_ptr >= arena.start) {
        Header *prev_hdr = (Header *)prev_ptr;
        // We need to verify that prev_hdr is actually a valid header pointing to this block
        // The end of the previous block should be exactly the start of this block
        uint8_t *prev_end = (uint8_t *)prev_hdr + sizeof(Header) + prev_hdr->size;
        if (prev_end == (uint8_t *)hdr && prev_hdr->free) {
            // Merge
            prev_hdr->size += sizeof(Header) + hdr->size;
            // We should update the pointer to point to the previous header
            // But the caller should handle this
        }
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) {
        return NULL;
    }
    
    // We need space for header + n bytes of data
    // But the data must be aligned.
    // Let's find a free block that has enough space.
    
    // Strategy: scan through all blocks.
    // If we find a free block, check if it's large enough.
    // If yes, allocate from it.
    
    uint8_t *ptr = arena.start;
    while (ptr < arena.end) {
        Header *hdr = (Header *)ptr;
        if (hdr->free) {
            // Check if this free block is large enough
            uint8_t *data_ptr = align_up(ptr + sizeof(Header));
            size_t available = (size_t)(ptr + sizeof(Header) + hdr->size - data_ptr);
            
            if (available >= n) {
                // Allocate
                mark_used(hdr);
                
                // If there's extra space, split the block
                size_t used_data = n;
                size_t total_used = (size_t)(data_ptr - ptr) + used_data;
                size_t remaining = hdr->size - (total_used - sizeof(Header));
                
                // Wait, hdr->size is the data size.
                // Let's re-calculate.
                // ptr points to header.
                // data_ptr points to start of data.
                // The space from ptr to data_ptr includes header and alignment padding.
                // The space from data_ptr to data_ptr + n is used data.
                // The remaining space is from data_ptr + n to ptr + sizeof(Header) + hdr->size.
                
                size_t end_of_block = (size_t)(ptr + sizeof(Header) + hdr->size);
                size_t end_of_data = (size_t)(data_ptr + n);
                
                if (end_of_data < end_of_block) {
                    // Split
                    size_t new_block_size = end_of_block - end_of_data;
                    // But we need to ensure the new block starts at an aligned address?
                    // The new block's header will be at end_of_data.
                    // The data of the new block will be at align_up(end_of_data + sizeof(Header)).
                    // This is fine, the alignment is handled when allocating from it.
                    
                    Header *new_hdr = (Header *)(data_ptr + n);
                    new_hdr->size = new_block_size - sizeof(Header);
                    new_hdr->free = true;
                    
                    // Update current header size
                    hdr->size = (size_t)(data_ptr - ptr) + n - sizeof(Header);
                    // Wait, hdr->size should be the data size.
                    // Let's redefine: hdr->size is the size of the data area.
                    // So for the current block, the data area is n bytes.
                    // But we also have alignment padding.
                    // Let's just say hdr->size is the total size of the block after the header.
                    // So hdr->size = end_of_block - (ptr + sizeof(Header)).
                    
                    // Let's restart with a clearer model.
                    // Model:
                    // Block: [Header][Data]
                    // Header: { size_t size; bool free; }
                    // size: number of bytes in Data.
                    // Data must be aligned to max_align_t relative to the start of the buffer?
                    // No, Data must be aligned to max_align_t in absolute terms.
                    
                    // So when we allocate, we find a free block.
                    // We check if align_up(ptr + sizeof