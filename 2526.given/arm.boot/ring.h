#ifndef _RING_H
#include "stdint.h"
#define _RING_H

#ifndef MAX_CHARS
#define MAX_CHARS 512
#endif

typedef uint8_t bool_t;

typedef struct ring {
     volatile uint32_t head;
     volatile uint32_t tail;
     volatile uint8_t buffer[MAX_CHARS];
} ring_t;

bool_t ring_is_empty(ring_t* ring);
bool_t ring_is_full(ring_t* ring);
void ring_put(ring_t* ring, uint8_t byte);
uint8_t ring_get(ring_t* ring);

#endif /* RING_H_ */