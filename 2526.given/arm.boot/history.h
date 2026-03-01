#ifndef _HISTORY_H
#define _HISTORY_H 

#include "stdint.h"
#include "stddef.h"

#define HISTORY_SIZE 5

extern char history[HISTORY_SIZE][80];
extern int history_idx;


void add_to_history(char* new_line);

void copy_from_history(char* dest_line);

#endif /* _HISTORY_H */
