#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define MIN_BLOCK_SIZE (ALIGN_UP(sizeof(struct block_header)))

struct block_header {
    size_t size;
    struct block_header *next;
    struct block_header *prev;
};

static struct block_header *free_list_head = NULL;
static void *arena_buffer = NULL;
static size_t arena_size = 0;

void arena_init(void *buf, size_t size) {
    if (!buf || size < MIN_BLOCK_SIZE) {
        free_list_head = NULL;
        arena_buffer = NULL;
        arena_size = 0;
        return;
    }

    arena_buffer = buf;
    arena_size = size;

    struct block_header *initial_block = (struct block_header *)buf;
    initial_block->size = size - sizeof(struct block_header);
    initial_block->next = NULL;
    initial_block->prev = NULL;

    free_list_head = initial_block;
}

static void remove_from_free_list(struct block_header *block) {
    if (block->prev) {
        block->prev->next = block->next;
    } else {
        free_list_head = block->next;
    }

    if (block->next) {
        block->next->prev = block->prev;
    }
}

static void add_to_free_list(struct block_header *) {
    /* Not used in this implementation */
}

static void coalesce_block(struct block_header *block) {
    /* Coalesce with next block if it's free */
    struct block_header *next_block = (struct block_header *)((char *)block + sizeof(struct block_header) + block->size);
    if ((char *)next_block < (char *)arena_buffer + arena_size &&
        next_block->size != 0 &&
        next_block->size <= arena_size) {
        /* Check if next block is in free list */
        struct block_header *current = free_list_head;
        while (current) {
            if (current == next_block) {
                /* Merge with next */
                block->size += sizeof(struct block_header) + next_block->size;
                remove_from_free_list(next_block);
                break;
            }
            current = current->next;
        }
    }

    /* Coalesce with previous block if it's free */
    if (block->prev) {
        struct block_header *prev_block = block->prev;
        struct block_header *current = free_list_head;
        while (current) {
            if (current == prev_block) {
                /* Merge with previous */
                prev_block->size += sizeof(struct block_header) + block->size;
                remove_from_free_list(block);
                block = prev_block;
                break;
            }
            current = current->next;
        }
    }
}

void *arena_alloc(size_t n) {
    if (n == 0 || !arena_buffer) {
        return NULL;
    }

    size_t aligned_size = ALIGN_UP(n);
    size_t total_size = aligned_size + sizeof(struct block_header);

    struct block_header *current = free_list_head;
    struct block_header *best_fit = NULL;
    struct block_header *best_prev = NULL;

    /* Find best fit block */
    while (current) {
        if (current->size >= aligned_size) {
            if (!best_fit || current->size < best_fit->size) {
                best_fit = current;
                best_prev = current->prev;
            }
        }
        current = current->next;
    }

    if (!best_fit) {
        return NULL;
    }

    /* Remove from free list */
    remove_from_free_list(best_fit);

    /* Split block if necessary */
    if (best_fit->size > aligned_size + MIN_BLOCK_SIZE) {
        struct block_header *new_block = (struct block_header *)((char *)best_fit + sizeof(struct block_header) + aligned_size);
        new_block->size = best_fit->size - aligned_size - sizeof(struct block_header);
        new_block->next = free_list_head;
        new_block->prev = NULL;
        if (free_list_head) {
            free_list_head->prev = new_block;
        }
        free_list_head = new_block;

        best_fit->size = aligned_size;
    }

    return (char *)best_fit + sizeof(struct block_header);
}

void arena_free(void *p) {
    if (!p || !arena_buffer) {
        return;
    }

    struct block_header *block = (struct block_header *)((char *)p - sizeof(struct block_header));

    /* Add to free list */
    block->next = free_list_head;
    block->prev = NULL;
    if (free_list_head) {
        free_list_head->prev = block;
    }
    free_list_head = block;

    /* Coalesce with neighbors */
    coalesce_block(block);
}

void *arena_realloc(void *p, size_t n) {
    if (!p) {
        return arena_alloc(n);
    }

    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    if (!arena_buffer) {
        return NULL;
    }

    struct block_header *block = (struct block_header *)((char *)p - sizeof(struct block_header));
    size_t aligned_size = ALIGN_UP(n);

    /* If new size is smaller or equal, no need to reallocate */
    if (aligned_size <= block->size) {
        return p;
    }

    /* Check if next block is free and can be merged */
    struct block_header *next_block = (struct block_header *)((char *)block + sizeof(struct block_header) + block->size);
    if ((char *)next_block < (char *)arena_buffer + arena_size) {
        struct block_header *current = free_list_head;
        while (current) {
            if (current == next_block && current->size + block->size + sizeof(struct block_header) >= aligned_size) {
                /* Grow into next block */
                size_t combined_size = block->size + sizeof(struct block_header) + current->size;
                remove_from_free_list(current);

                if (combined_size > aligned_size + MIN_BLOCK_SIZE) {
                    struct block_header *new_block = (struct block_header *)((char *)block + sizeof(struct block_header) + aligned_size);
                    new_block->size = combined_size - aligned_size - sizeof(struct block_header);
                    new_block->next = free_list_head;
                    new_block->prev = NULL;
                    if (free_list_head) {
                        free_list_head->prev = new_block;
                    }
                    free_list_head = new_block;

                    block->size = aligned_size;
                } else {
                    block->size = combined_size;
                }

                return p;
            }
            current = current->next;
        }
    }

    /* Need to allocate new block and copy */
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) {
        return NULL;
    }

    size_t copy_size = block->size < aligned_size ? block->size : aligned_size;
    char *src = (char *)p;
    char *dst = (char *)new_ptr;
    for (size_t i = 0; i < copy_size; i++) {
        dst[i] = src[i];
    }

    arena_free(p);
    return new_ptr;
}