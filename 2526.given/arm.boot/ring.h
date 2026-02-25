#ifndef _RING_H
#include "stdint.h"
#define _RING_H

#ifndef MAX_CHARS
#define MAX_CHARS 512
#endif

typedef uint8_t bool_t;

extern volatile uint32_t head;
extern volatile uint32_t tail;
extern volatile uint8_t buffer[MAX_CHARS];

bool_t ring_is_empty();
bool_t ring_is_full();
void ring_put(uint8_t byte);
uint8_t ring_get();

#endif /* RING_H_ */