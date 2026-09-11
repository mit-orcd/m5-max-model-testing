#include <stddef.h>
#include <string.h>
#include <stdint.h>

#define ARENA_MAGIC 0xA5E11A5Eu

typedef struct ArenaHeader {
    size_t size;
    int is_free;
    struct ArenaHeader *next;
    struct ArenaHeader *prev;
    uint32_t magic;
} ArenaHeader;

typedef struct ArenaState {
    void *buf;
    size_t size;
    ArenaHeader *free_list;
    ArenaHeader *start;
} ArenaState;

static ArenaState g_arena;

#define ALIGN_UP(n, a) (((n) + ((a) - 1)) & ~((a) - 1))

static size_t header_size(void) {
    return ALIGN_UP(sizeof(ArenaHeader), _Alignof(max_align_t));
}

static ArenaHeader *make_header(void *ptr, size_t size, int is_free) {
    ArenaHeader *h = (ArenaHeader *)ptr;
    h->size = size;
    h->is_free = is_free;
    h->magic = ARENA_MAGIC;
    return h;
}

static void insert_free_after(ArenaHeader *after, ArenaHeader *h) {
    h->next = after->next;
    h->prev = after;
    after->next = h;
    if (h->next)
        h->next->prev = h;
    else
        g_arena.free_list = h;
    /* update free_list if needed */
    if (g_arena.free_list == after)
        g_arena.free_list = h;
}

static void remove_free(ArenaHeader *h) {
    if (h->prev)
        h->prev->next = h->next;
    else
        g_arena.free_list = h->next;
    if (h->next)
        h->next->prev = h->prev;
    h->prev = NULL;
    h->next = NULL;
}

static void arena_init(void *buf, size_t size) {
    g_arena.buf = buf;
    g_arena.size = size;
    g_arena.free_list = NULL;
    g_arena.start = NULL;

    if (!buf || size < header_size()) {
        g_arena.start = NULL;
        g_arena.free_list = NULL;
        return;
    }

    size_t hs = header_size();
    size_t data_size = size - hs;
    ArenaHeader *h = make_header(buf, data_size, 1);
    h->prev = NULL;
    h->next = NULL;
    g_arena.start = h;
    g_arena.free_list = h;
}

static ArenaHeader *find_free_block(size_t needed) {
    ArenaHeader *h = g_arena.free_list;
    while (h) {
        if (h->size >= needed)
            return h;
        h = h->next;
    }
    return NULL;
}

static void split_block(ArenaHeader *h, size_t alloc_size) {
    size_t hs = header_size();
    size_t remaining = h->size - alloc_size;
    if (remaining >= hs + _Alignof(max_align_t)) {
        ArenaHeader *new_h = make_header((char *)h + hs + alloc_size, remaining - hs, 1);
        h->size = alloc_size;
        /* insert new_h after h in the free list if h is free */
        if (h->is_free) {
            remove_free(h);
            h->is_free = 0;
            /* insert new_h in free list at h's position */
            new_h->prev = h->prev;
            new_h->next = h->next;
            if (h->prev)
                h->prev->next = new_h;
            else
                g_arena.free_list = new_h;
            if (new_h->next)
                new_h->next->prev = new_h;
        } else {
            /* h is allocated, new_h goes into free list */
            new_h->prev = h->prev;
            new_h->next = h->next;
            if (h->prev)
                h->prev->next = new_h;
            else
                g_arena.free_list = new_h;
            if (new_h->next)
                new_h->next->prev = new_h;
        }
        /* Also update the physical next pointer */
        /* We need to track physical adjacency separately */
    } else {
        h->size = alloc_size;
    }
}

/* We need to track physical next/prev separately from free list links */
/* Let's redesign: use a single linked list for physical order, and free list for free blocks */

/* Redesigning with physical links */
typedef struct PhysBlock {
    size_t size;
    int is_free;
    struct PhysBlock *phys_next;
    struct PhysBlock *phys_prev;
    uint32_t magic;
} PhysBlock;

static PhysBlock *g_phys_start;
static PhysBlock *g_free_head;
static void *g_buf;
static size_t g_size;

static size_t phys_header_size(void) {
    return ALIGN_UP(sizeof(PhysBlock), _Alignof(max_align_t));
}

static void phys_insert_free(PhysBlock *b) {
    b->phys_prev->phys_next = b;
    /* Insert into free list */
    b->is_free = 1;
    /* Simple: prepend to free list */
    b->phys_next = g_free_head;
    b->phys_prev = g_free_head ? g_free_head->phys_prev : NULL;
    /* Actually, let's use a separate free list with its own links */
    /* For simplicity, let's just use the phys links for free list too, but that conflicts */
    
    /* Let me use a different approach: free list uses the same nodes but we need separate links */
    /* I'll add free_next and free_prev */
}

/* Let me restart with a cleaner design */

typedef struct Block {
    size_t size;
    int is_free;
    struct Block *next;  /* physical next */
    struct Block *prev;  /* physical prev */
    struct Block *free_next; /* next in free list */
    struct Block *free_prev; /* prev in free list */
    uint32_t magic;
} Block;

static Block *g_start;
static Block *g_free_list;
static void *g_arena_buf;
static size_t g_arena_size;

static size_t blk_header_size(void) {
    return ALIGN_UP(sizeof(Block), _Alignof(max_align_t));
}

static void free_list_insert(Block *b) {
    b->is_free = 1;
    b->free_prev = NULL;
    b->free_next = g_free_list;
    if (g_free_list)
        g_free_list->free_prev = b;
    g_free_list = b;
}

static void free_list_remove(Block *b) {
    b->is_free = 0;
    if (b->free_prev)
        b->free_prev->free_next = b->free_next;
    else
        g_free_list = b->free_next;
    if (b->free_next)
        b->free_next->free_prev = b->free_prev;
    b->free_prev = NULL;
    b->free_next = NULL;
}

static void arena_init(void *buf, size_t size) {
    g_arena_buf = buf;
    g_arena_size = size;
    g_start = NULL;
    g_free_list = NULL;

    if (!buf || size < blk_header_size()) {
        g_start = NULL;
        g_free_list = NULL;
        return;
    }

    size_t hs = blk_header_size();
    size_t data = size - hs;
    Block *b = (Block *)buf;
    b->size = data;
    b->is_free = 1;
    b->next = NULL;
    b->prev = NULL;
    b->free_next = NULL;
    b->free_prev = NULL;
    b->magic = ARENA_MAGIC;
    g_start = b;
    g_free_list = b;
}

static void *arena_alloc(size_t n) {
    if (n == 0)
        return NULL;
    if (!g_start)
        return NULL;

    size_t hs = blk_header_size();
    size_t needed = ALIGN_UP(n, _Alignof(max_align_t)) + hs;

    Block *b = g_free_list;
    Block *chosen = NULL;
    while (b) {
        if (b->size >= needed) {
            chosen = b;
            break;
        }
        b = b->free_next;
    }

    if (!chosen)
        return NULL;

    free_list_remove(chosen);

    size_t remaining = chosen->size - needed;
    if (remaining >= hs + _Alignof(max_align_t)) {
        Block *new_b = (Block *)((char *)chosen + hs + needed);
        new_b->size = remaining - hs;
        new_b->is_free = 1;
        new_b->next = chosen->next;
        new_b->prev = chosen;
        if (chosen->next)
            chosen->next->prev = new_b;
        else
            /* chosen was last, now new_b is last */
            ;
        chosen->next = new_b;
        chosen->size = needed;
        free_list_insert(new_b);
    } else {
        chosen->size = needed;
    }

    return (char *)chosen + hs;
}

static void arena_free(void *p) {
    if (!p)
        return;
    if (!g_start)
        return;

    size_t hs = blk_header_size();
    Block *b = (Block *)((char *)p - hs);

    if (b->magic != ARENA_MAGIC)
        return;
    if (!b->is_free) {
        free_list_insert(b);

        /* Coalesce with next */
        if (b->next && b->next->is_free) {
            Block *nb = b->next;
            free_list_remove(nb);
            b->size += hs + nb->size;
            b->next = nb->next;
            if (nb->next)
                nb->next->prev = b;
        }
        /* Coalesce with prev */
        if (b->prev && b->prev->is_free) {
            Block *pb = b->prev;
            free_list_remove(pb);
            pb->size += hs + b->size;
            pb->next = b->next;
            if (b->next)
                b->next->prev = pb;
            /* b is now absorbed, free_list already has pb */
        }
    }
}

static void *arena_realloc(void *p, size_t n) {
    if (!p)
        return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    if (!g_start)
        return NULL;

    size_t hs = blk_header_size();
    Block *b = (Block *)((char *)p - hs);

    if (b->magic != ARENA_MAGIC)
        return NULL;

    size_t old_data = b->size - hs;
    size_t new_data = ALIGN_UP(n, _Alignof(max_align_t));

    if (new_data <= old_data) {
        /* Shrink: may split */
        size_t needed = new_data + hs;
        size_t remaining = b->size - needed;
        if (remaining >= hs + _Alignof(max_align_t)) {
            Block *new_b = (Block *)((char *)b + hs + new_data);
            new_b->size = remaining - hs;
            new_b->is_free = 1;
            new_b->next = b->next;
            new_b->prev = b;
            if (b->next)
                b->next->prev = new_b;
            b->next = new_b;
            b->size = needed;
            free_list_insert(new_b);
        } else {
            b->size = needed;
        }
        return p;
    }

    /* Need to grow */
    size_t needed = new_data + hs;
    size_t deficit = needed - b->size;

    /* Check if next block is free and can absorb */
    if (b->next && b->next->is_free) {
        Block *nb = b->next;
        if (hs + nb->size >= deficit) {
            free_list_remove(nb);
            b->size += hs + nb->size;
            b->next = nb->next;
            if (nb->next)
                nb->next->prev = b;
            /* Split if there's leftover */
            size_t remaining = b->size - needed;
            if (remaining >= hs + _Alignof(max_align_t)) {
                Block *new_b = (Block *)((char *)b + hs + new_data);
                new_b->size = remaining - hs;
                new_b->is_free = 1;
                new_b->next = b->next;
                new_b->prev = b;
                if (b->next)
                    b->next->prev = new_b;
                b->next = new_b;
                b->size = needed;
                free_list_insert(new_b);
            }
            return p;
        }
    }

    /* Cannot grow in place, allocate new and copy */
    void *new_p = arena_alloc(n);
    if (!new_p)
        return NULL;
    size_t copy_len = n < old_data ? n : old_data;
    memcpy(new_p, p, copy_len);
    arena_free(p);
    return new_p;
}