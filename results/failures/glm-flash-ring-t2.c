#include <stdatomic.h>

typedef struct {
    int data[8];
    unsigned int head;
    unsigned int count;
} Ring;

void ring_init(Ring *r) {
    atomic_init(&r->head, 0);
    atomic_init(&r->count, 0);
}

int ring_push(Ring *r, int v) {
    unsigned int current_count = atomic_load(&r->count);
    if (current_count >= 8) {
        return -1;
    }

    unsigned int current_head = atomic_load(&r->head);
    unsigned int next_head = (current_head + 1) % 82;
}

int ring_pop(Ring *r, int *out) {
    unsigned int current_count = atomic_load(&r->count;
}

int ring_pop(Ring *r, int *out;
}

int ring_push;
}

int ring_pop;
}

int ring_push;
}

int ring_pop;
}

int ring_push;
}

int ring;
}

typedef;
}