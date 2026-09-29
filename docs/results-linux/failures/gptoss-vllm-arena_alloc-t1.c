```c
#include <stddef.h>
#include <stdalign.h>
#include <string.h>

/* Block header structure.  The header is aligned to max_align_t. */
typedef struct block_header {
    size_t          size;   /* total size of the block (header + payload + footer) */
    int             free;   /* 1 if free, 0 if allocated */
    struct block_header *next; /* next free block in the free list */
    struct block_header *prev; /* previous free block in the free list */
} block_header;

/* Global arena state */
static void            *arena_start   = NULL;
static size_t           arena_size    = 0;
static block_header    *free_list_head = NULL;

/* Size of the header and footer, aligned to max_align_t */
static const size_t header_size = alignof(max_align_t) < sizeof(block_header) ?
                                 sizeof(block_header) : alignof(max_align_t);
static const size_t footer_size = sizeof(size_t);

/* Write a footer containing the block size at the end of the block. */
static void write_footer(block_header *h)
{
    size_t *footer = (size_t *)((char *)h + h->size - footer_size);
    *footer = h->size;
}

/* Remove a block from the free list. */
static void remove_from_free_list(block_header *h)
{
    if (h->prev)
        h->prev->next = h->next;
    else
        free_list_head = h->next;
    if (h->next)
        h->next->prev = h->prev;
    h->next = h->prev = NULL;
}

/* Insert a block at the head of the free list. */
static void insert_into_free_list(block_header *h)
{
    h->next = free_list_head;
    h->prev = NULL;
    if (free_list_head)
        free_list_head->prev = h;
    free_list_head = h;
}

/* Initialise the arena with the supplied buffer. */
void arena_init(void *buf, size_t size)
{
    arena_start   = buf;
    arena_size    = size;
    free_list_head = (block_header *)buf;

    free_list_head->size = size;
    free_list_head->free = 1;
    free_list_head->next = NULL;
    free_list_head->prev = NULL;
    write_footer(free_list_head);
}

/* Allocate a block of at least n bytes. */
void *arena_alloc(size_t n)
{
    if (n == 0)
        return NULL;

    /* Align the requested size to max_align_t. */
    size_t aligned_n = (n + (alignof(max_align_t) - 1)) & ~(alignof(max_align_t) - 1);
    size_t needed = header_size + aligned_n + footer_size;

    block_header *h = free_list_head;
    while (h) {
        if (h->size >= needed) {
            /* Found a suitable block. */
            if (h->size == needed) {
                /* Exact fit. */
                remove_from_free_list(h);
                h->free = 0;
                /* Footer already correct. */
                return (void *)((char *)h + header_size);
            } else {
                /* Split the block. */
                remove_from_free_list(h);

                size_t remaining = h->size - needed;
                if (remaining >= header_size + footer_size) {
                    /* Create a new free block for the remaining space. */
                    block_header *new_free = (block_header *)((char *)h + needed);
                    new_free->size = remaining;
                    new_free->free =