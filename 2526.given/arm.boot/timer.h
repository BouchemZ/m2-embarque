#ifndef _TIMER_H_
#define _TIMER_H_ 

#include "stdint.h"
#include "stddef.h"
#include "main.h"

#define TIMER0 (void*)0x101E2000
#define TIMER1 (void*)0x101E2020
#define TIMER2 (void*)0x101E3000
#define TIMER3 (void*)0x101E3020


#define TIMER_LOAD 0x00
#define TIMER_VALUE 0x04
#define TIMER_CONTROL 0x08
#define TIMER_INTCLR 0x0C
#define TIMER_RIS 0x10
#define TIMER_MIS 0x14


#define TIMER_ENABLE (1<<7)
#define TIMER_MODE_PERIODIC (1<<6)
#define TIMER_SIZE_32 (1<<1)
#define TIMER_IRQ_ENABLE (1<<5)


void timer_init(void* timer);

void timer_irq_enable(void* timer);

void timer_irq_disable(void* timer);

void timer_clear_interrupt(void* timer);




#endif /* _TIMER_H_ */
