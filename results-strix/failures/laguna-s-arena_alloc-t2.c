#include <stddef.h>
#include <stdalign.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define MIN_BLOCK_SIZE (ALIGN_UP(sizeof(struct block_header)))

struct block_header {
    struct block_header *next;
    struct block_header *prev;
    size_t size;
    int free;
};

static struct block_header *free_list_head = NULL;
static struct block_header *buffer_start = NULL;
static size_t buffer_size = 0;

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

static void add_to_free_list(struct block_header *block) {
    block->prev = NULL;
    block->next = free_list_head;
    if (free_list_head) {
        free_list_head->prev = block;
    }
    free_list_head = block;
}

static void merge_with_neighbor(struct block_header *block) {
    struct block_header *current = buffer_start;
    while (current) {
        struct block_header *next = (struct block_header *)((char *)current + current->size);
        if (next == block && !block->free) {
            // Merge with previous if it's free
            if (current->free) {
                current->size += block->size;
                remove_from_free_list(current);
                block = current;
            }
            break;
        }
        if (current == block) {
            // Check next block
            if ((char *)next < (char *)buffer_start + buffer_size && !next->free) {
                block->size += next->size;
                remove_from_free_list(next);
            }
            break;
        }
        current = next;
    }
}

void arena_init(void *buf, size_t size) {
    buffer_start = (struct block_header *)buf;
    buffer_size = size;
    free_list_head = NULL;

    if (size < MIN_BLOCK_SIZE) {
        return;
    }

    struct block_header *header = buffer_start;
    header->size = size;
    header->free = 1;
    header->next = NULL;
    header->prev = NULL;
    free_list_head = header;
}

void *arena_alloc(size_t n) {
    if (n == 0) {
        return NULL;
    }

    size_t aligned_n = ALIGN_UP(n);
    if (aligned_n < MIN_BLOCK_SIZE) {
        aligned_n = MIN_BLOCK_SIZE;
    }

    struct block_header *current = free_list_head;
    while (current) {
        if (current->free && current->size >= aligned_n) {
            if (current->size > aligned_n + MIN_BLOCK_SIZE) {
                // Split the block
                struct block_header *new_block = (struct block_header *)((char *)current + aligned_n);
                new_block->size = current->size - aligned_n;
                new_block->free = 1;
                new_block->next = current->next;
                new_block->prev = current->prev;

                if (current->prev) {
                    current->prev->next = new_block;
                } else {
                    free_list_head = new_block;
                }
                if (current->next) {
                    current->next->prev = new_block;
                }

                current->size = aligned_n;
                current->free = 0;
                return (char *)current + MIN_BLOCK_SIZE;
            } else {
                current->free = 0;
                remove_from_free_list(current);
                return (char *)current + MIN_BLOCK_SIZE;
            }
        }
        current = current->next;
    }

    return NULL;
}

void arena_free(void *p) {
    if (!p) {
        return;
    }

    struct block_header *block = (struct block_header *)((char *)p - MIN_BLOCK_SIZE);
    block->free = 1;
    add_to_free_list(block);

    // Try to merge with neighbors
    struct block_header *current = buffer_start;
    while (current) {
        if (current->free) {
            struct block_header *next = (struct block_header *)((char *)current + current->size);
            if ((char *)next < (char *)buffer_start + buffer_size && next->free) {
                current->size += next->size;
                remove_from_free_list(next);
            }
        }
        current = (struct block_header *)((char *)current + current->size);
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    struct block_header *block = (struct block_header *)((char *)p - MIN_BLOCK_SIZE);
    size_t old_size = block->size - MIN_BLOCK_SIZE;

    if (n <= old_size) {
        return p;
    }

    size_t aligned_n = ALIGN_UP(n);
    if (aligned_n < MIN_BLOCK_SIZE) {
        aligned_n = MIN_BLOCK_SIZE;
    }

    struct block_header *next = (struct block_header *)((char *)block + block->size);
    if ((char *)next < (char *)buffer_start + buffer_size && next->free && block->size + next->size >= aligned_n + MIN_BLOCK_SIZE) {
        // Expand into the next block
        block->size += next->size;
        remove_from_free_list(next);
        return p;
    }

    // Allocate new block and copy data
    void *new_p = arena_alloc(n);
    if (!new_p) {
        return NULL;
    }

    size_t copy_size = old_size < n ? old_size : n;
    char *dst = (char *)new_p;
    char *src = (char *)p;
    for (size_t i = 0; i < copy_size; i++) {
        dst[i] = src[i];
    }

    arena_free(p);
    return new_p;
}