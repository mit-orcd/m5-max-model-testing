#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* Internal header structure stored at the start of each block */
typedef struct block_header {
    size_t size; /* Size of the usable data area (excluding header) */
    size_t flags; /* Bit 0: 1 if free, 0 if allocated */
    struct block_header *next; /* Pointer to next block header in the arena (or NULL) */
    struct block_header *prev; /* Pointer to previous block header in the arena (or NULL) */
} block_header;

/* Arena state stored at the beginning of the buffer */
typedef struct arena_state {
    size_t total_size; /* Total size of the buffer passed to arena_init */
    size_t state_size; /* Size of this state structure, aligned */
    block_header *head; /* First block header */
    block_header *tail; /* Last block header */
} arena_state;

#define ARENA_STATE_OFFSET sizeof(arena_state)

/* Helper to align a size up to max_align_t */
static size_t align_up(size_t size) {
    size_t alignment = _Alignof(max_align_t);
    return (size + alignment - 1) & ~(alignment - 1);
}

/* Helper to get the pointer to the data from a header */
static void *header_to_data(block_header *h) {
    return (void *)((char *)h + sizeof(block_header));
}

/* Helper to get the header from a data pointer */
static block_header *data_to_header(void *p) {
    return (block_header *)((char *)p - sizeof(block_header));
}

/* Helper to get the arena state from the buffer start */
static arena_state *get_state(void *buf) {
    return (arena_state *)buf;
}

/* Helper to get the end of the arena */
static void *arena_end(void *buf, size_t total_size) {
    return (void *)((char *)buf + total_size);
}

/* Initialize the arena */
void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(arena_state)) {
        /* Cannot initialize with too small a buffer */
        return;
    }

    arena_state *state = get_state(buf);
    size_t state_sz = align_up(sizeof(arena_state));
    
    /* Check if there is room for at least one minimal block */
    size_t header_sz = sizeof(block_header);
    size_t min_block = align_up(header_sz);
    
    if (size < state_sz + min_block) {
        /* Initialize state but mark as unusable / empty */
        state->total_size = size;
        state->state_size = state_sz;
        state->head = NULL;
        state->tail = NULL;
        return;
    }

    state->total_size = size;
    state->state_size = state_sz;

    /* Create the initial free block spanning the rest of the buffer */
    block_header *first = (block_header *)((char *)buf + state_sz);
    size_t usable_size = size - state_sz - header_sz;
    
    /* Ensure usable size is aligned properly for the next block if needed, 
       but here we just make one big block. */
    first->size = usable_size;
    first->flags = 1; /* Free */
    first->next = NULL;
    first->prev = NULL;

    state->head = first;
    state->tail = first;
}

/* Allocate memory from the arena */
void *arena_alloc(size_t n) {
    if (n == 0) {
        return NULL;
    }

    /* We need to find the arena state. 
       The problem statement implies arena functions work on a global or passed state.
       However, the signature doesn't take the buffer. 
       Standard arena allocators usually store the state globally or pass it.
       
       Looking at the signatures:
       void arena_init(void *buf, size_t size);
       void *arena_alloc(size_t n);
       
       This implies a global state or a way to find the state. 
       Since no global variable is allowed/mentioned, and arena_init takes a buffer,
       typically such APIs expect a single global arena or the state is found via 
       a global pointer. 
       
       However, the constraint "Write a complete fixed-buffer allocator" with these 
       specific signatures usually implies a global static variable to hold the current 
       arena state pointer. Without it, arena_alloc cannot know which buffer to use.
       
       Let's assume a global static variable to hold the arena state.
    */
    
    static arena_state *current_arena = NULL;
    
    /* If no arena is initialized, return NULL */
    if (!current_arena) {
        return NULL;
    }

    /* Check if the current arena is valid */
    if (current_arena->total_size < current_arena->state_size) {
        return NULL;
    }

    /* Align the requested size */
    size_t aligned_n = align_up(n);
    
    /* We need a block of size at least aligned_n + sizeof(block_header) */
    size_t needed = aligned_n + sizeof(block_header);

    block_header *curr = current_arena->head;
    void *buf_ptr = (void *)current_arena; /* The start of the buffer is the state */
    
    /* Traverse the free list to find a suitable block */
    while (curr != NULL) {
        if ((curr->flags & 1) && curr->size >= aligned_n) {
            /* Found a free block that is large enough */
            
            /* Split if necessary */
            size_t remaining = curr->size - aligned_n;
            
            /* We split if the remaining space can hold a new header + min data */
            size_t min_split_size = sizeof(block_header) + align_up(1); /* At least 1 byte data */
            
            if (remaining >= min_split_size) {
                /* Split the block */
                block_header *new_block = (block_header *)((char *)curr + sizeof(block_header) + aligned_n);
                
                /* Initialize new block */
                new_block->size = remaining - sizeof(block_header);
                new_block->flags = 1; /* Free */
                new_block->next = curr->next;
                new_block->prev = curr;
                
                if (curr->next) {
                    curr->next->prev = new_block;
                } else {
                    current_arena->tail = new_block;
                }
                
                curr->next = new_block;
                curr->size = aligned_n;
            }
            
            /* Mark as allocated */
            curr->flags &= ~1; /* Clear free bit */
            
            return header_to_data(curr);
        }
        curr = curr->next;
    }

    /* No suitable block found */
    return NULL;
}

/* Free memory */
void arena_free(void *p) {
    if (p == NULL) {
        return;
    }

    static arena_state *current_arena = NULL;
    
    /* We need to get the header from p */
    block_header *block = data_to_header(p);
    
    /* Basic sanity check: is this block within a valid arena? 
       We rely on the flags being correctly set. */
    
    /* Mark as free */
    block->flags |= 1;
    
    /* Try to merge with next neighbor */
    if (block->next && (block->next->flags & 1)) {
        /* Merge with next */
        size_t merged_size = block->size + sizeof(block_header) + block->next->size;
        
        block->size = merged_size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        } else {
            current_arena->tail = block;
        }
    }
    
    /* Try to merge with previous neighbor */
    if (block->prev && (block->prev->flags & 1)) {
        /* Merge with prev */
        block_header *prev = block->prev;
        size_t merged_size = prev->size + sizeof(block_header) + block->size;
        
        prev->size = merged_size;
        prev->next = block->next;
        if (prev->next) {
            prev->next->prev = prev;
        } else {
            current_arena->tail = prev;
        }
    }
}

/* Reallocate memory */
void *arena_realloc(void *p, size_t n) {
    if (p == NULL) {
        return arena_alloc(n);
    }
    
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    static arena_state *current_arena = NULL;
    if (!current_arena) {
        return NULL;
    }

    block_header *block = data_to_header(p);
    size_t old_size = block->size;
    size_t aligned_n = align_up(n);

    /* Case 1: Requested size is same or smaller */
    if (aligned_n <= old_size) {
        /* We can shrink, but usually we just return p. 
           If we want to reclaim space, we'd need to split. 
           For simplicity and correctness per "preserves contents", returning p is fine.
           The block remains allocated with the old size. */
        return p;
    }

    /* Case 2: Try to grow into the next block if it is free */
    if (block->next && (block->next->flags & 1)) {
        size_t available = old_size + sizeof(block_header) + block->next->size;
        
        if (available >= aligned_n) {
            /* We can expand into the next block */
            
            /* Check if we should split the remainder */
            size_t remaining = available - aligned_n;
            size_t min_split_size = sizeof(block_header) + align_up(1);
            
            if (remaining >= min_split_size) {
                /* Split the combined space */
                block_header *new_block = (block_header *)((char *)block + sizeof(block_header) + aligned_n);
                
                new_block->size = remaining - sizeof(block_header);
                new_block->flags = 1; /* Free */
                new_block->next = block->next->next;
                new_block->prev = block;
                
                if (block->next->next) {
                    block->next->next->prev = new_block;
                } else {
                    current_arena->tail = new_block;
                }
                
                block->next = new_block;
                block->size = aligned_n;
            } else {
                /* Consume the entire next block */
                block->size = available;
                block->next = block->next->next;
                if (block->next) {
                    block->next->prev = block;
                } else {
                    current_arena->tail = block;
                }
            }
            return p;
        }
    }

    /* Case 3: Cannot grow in place, allocate new, copy, free old */
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) {
        return NULL;
    }

    size_t copy_size = (n < old_size) ? n : old_size;
    memcpy(new_ptr, p, copy_size);
    
    arena_free(p);
    
    return new_ptr;
}

/* Helper to set the global arena state, called by arena_init */
void arena_init(void *buf, size_t size);
static arena_state *current_arena = NULL;

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(arena_state)) {
        current_arena = NULL;
        return;
    }

    arena_state *state = (arena_state *)buf;
    size_t state_sz = align_up(sizeof(arena_state));
    
    /* Check if there is room for at least one minimal block */
    size_t header_sz = sizeof(block_header);
    
    /* We need at least header_sz for the first block */
    if (size < state_sz + header_sz) {
        state->total_size = size;
        state->state_size = state_sz;
        state->head = NULL;
        state->tail = NULL;
        current_arena = state;
        return;
    }

    state->total_size = size;
    state->state_size = state_sz;

    /* Create the initial free block spanning the rest of the buffer */
    block_header *first = (block_header *)((char *)buf + state_sz);
    size_t usable_size = size - state_sz - header_sz;
    
    first->size = usable_size;
    first->flags = 1; /* Free */
    first->next = NULL;
    first->prev = NULL;

    state->head = first;
    state->tail = first;
    
    current_arena = state;
}