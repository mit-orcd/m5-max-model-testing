#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN_UP(p, a) (((uintptr_t)(p) + (a) - 1) & ~((a) - 1))
#define BLOCK_HEADER_SIZE ALIGN_UP(sizeof(struct block_header), ALIGNMENT)

struct block_header {
    size_t size;           // total block size including header (0 if free)
    struct block_header *next;
    struct block_header *prev;
};

static struct block_header *head = NULL;
static void *arena_buffer = NULL;
static size_t arena_size = 0;

static void insert_free_block(struct block_header *blk) {
    if (!blk) return;
    blk->size = 0;
    
    struct block_header *cur = head;
    struct block_header *prev = NULL;
    
    while (cur && (uintptr_t)cur < (uintptr_t)blk) {
        prev = cur;
        cur = cur->next;
    }
    
    blk->next = cur;
    blk->prev = prev;
    
    if (prev) prev->next = blk;
    else head = blk;
    
    if (cur) cur->prev = blk;
}

static void remove_free_block(struct block_header *blk) {
    if (!blk || blk->size != 0) return;
    
    if (blk->prev) blk->prev->next = blk->next;
    else head = blk->next;
    
    if (blk->next) blk->next->prev = blk->prev;
}

void arena_init(void *buf, size_t size) {
    arena_buffer = buf;
    arena_size = size;
    head = NULL;
    
    if (!buf || size < BLOCK_HEADER_SIZE + ALIGNMENT) return;
    
    struct block_header *initial = (struct block_header *)ALIGN_UP((uintptr_t)buf + BLOCK_HEADER_SIZE, ALIGNMENT);
    size_t usable_size = size - ((uintptr_t)initial - (uintptr_t)buf);
    
    initial->size = 0;
    initial->next = NULL;
    initial->prev = NULL;
    head = initial;
}

void *arena_alloc(size_t n) {
    if (!arena_buffer || !head) return NULL;
    
    if (n == 0) return NULL;
    
    size_t total_size = BLOCK_HEADER_SIZE + ALIGN_UP(n, ALIGNMENT);
    total_size = total_size < BLOCK_HEADER_SIZE ? BLOCK_HEADER_SIZE : total_size;
    
    struct block_header *cur = head;
    while (cur) {
        if (cur->size == 0 && cur->size + BLOCK_HEADER_SIZE >= total_size) {
            size_t remaining = cur->size + BLOCK_HEADER_SIZE - total_size;
            
            remove_free_block(cur);
            
            if (remaining >= BLOCK_HEADER_SIZE + ALIGNMENT) {
                struct block_header *new_block = (struct block_header *)((char *)cur + total_size);
                new_block->size = remaining - BLOCK_HEADER_SIZE;
                new_block->next = cur->next;
                new_block->prev = cur->prev;
                if (cur->next) cur->next->prev = new_block;
                if (cur->prev) cur->prev->next = new_block;
                else head = new_block;
                
                cur->size = total_size;
                cur->next = NULL;
                cur->prev = NULL;
                insert_free_block(new_block);
            } else {
                cur->size = cur->size + BLOCK_HEADER_SIZE;
                cur->next = NULL;
                cur->prev = NULL;
            }
            
            return (void *)((char *)cur + BLOCK_HEADER_SIZE);
        }
        cur = cur->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (!p || !arena_buffer || !head) return;
    
    struct block_header *blk = (struct block_header *)((char *)p - BLOCK_HEADER_SIZE);
    
    if ((uintptr_t)blk < (uintptr_t)arena_buffer || 
        (uintptr_t)blk + BLOCK_HEADER_SIZE > (uintptr_t)arena_buffer + arena_size) {
        return;
    }
    
    if (blk->size == 0) return;
    
    size_t blk_size = blk->size;
    blk->size = 0;
    blk->next = NULL;
    blk->prev = NULL;
    
    insert_free_block(blk);
    
    struct block_header *next_blk = blk->next;
    if (next_blk && (uintptr_t)next_blk == (uintptr_t)blk + blk_size) {
        remove_free_block(blk);
        remove_free_block(next_blk);
        
        blk->size = blk_size + BLOCK_HEADER_SIZE + next_blk->size;
        blk->next = next_blk->next;
        if (next_blk->next) next_blk->next->prev = blk;
        insert_free_block(blk);
    }
    
    struct block_header *prev_blk = blk->prev;
    if (prev_blk && prev_blk->size == 0 && 
        (uintptr_t)blk == (uintptr_t)prev_blk + prev_blk->size) {
        remove_free_block(prev_blk);
        remove_free_block(blk);
        
        prev_blk->size = prev_blk->size + BLOCK_HEADER_SIZE + blk->size;
        prev_blk->next = blk->next;
        if (blk->next) blk->next->prev = prev_blk;
        insert_free_block(prev_blk);
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    
    struct block_header *blk = (struct block_header *)((char *)p - BLOCK_HEADER_SIZE);
    
    if ((uintptr_t)blk < (uintptr_t)arena_buffer || 
        (uintptr_t)blk + BLOCK_HEADER_SIZE > (uintptr_t)arena_buffer + arena_size) {
        return NULL;
    }
    
    size_t old_size = blk->size - BLOCK_HEADER_SIZE;
    size_t needed = ALIGN_UP(n, ALIGNMENT);
    size_t total_needed = BLOCK_HEADER_SIZE + needed;
    
    if (old_size >= needed) {
        return p;
    }
    
    struct block_header *next_blk = blk->next;
    if (next_blk && next_blk->size == 0 && 
        (uintptr_t)next_blk == (uintptr_t)blk + blk->size) {
        size_t combined_size = blk->size + BLOCK_HEADER_SIZE + next_blk->size;
        if (combined_size >= total_needed) {
            remove_free_block(next_blk);
            
            blk->size = combined_size;
            
            size_t remaining = combined_size - total_needed;
            if (remaining >= BLOCK_HEADER_SIZE + ALIGNMENT) {
                struct block_header *new_next = (struct block_header *)((char *)blk + total_needed);
                new_next->size = remaining - BLOCK_HEADER_SIZE;
                new_next->next = next_blk->next;
                new_next->prev = blk;
                if (next_blk->next) next_blk->next->prev = new_next;
                blk->next = new_next;
                insert_free_block(new_next);
            } else {
                blk->next = next_blk->next;
                if (next_blk->next) next_blk->next->prev = blk;
            }
            
            return p;
        }
    }
    
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) return NULL;
    
    memcpy(new_ptr, p, old_size < n ? old_size : n);
    arena_free(p);
    
    return new_ptr;
}