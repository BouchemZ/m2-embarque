/*
 * Copyright: Olivier Gruber (olivier dot gruber at acm dot org)
 *
 * This program is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * isr.h
 *
 *  Created on: Jan 21, 2021
 *      Author: ogruber
 */

#include "isr.h"

typedef struct handler {
    void (*callback)(void*);
    void* cookie;
} handler_t;

handler_t handlers[NIRQS];


// get the status of the interrupts
uint32_t vic_load_irqs(){
    return mmio_read32(VIC_BASE_ADDR, VICIRQSTATUS);
}

void isr_handler(){
    uint32_t irqs = vic_load_irqs();
    for(uint32_t i = 0; i < NIRQS; i++){
        handler_t* handler;
        handler = &handlers[i];
        if (irqs & (1<<i)){
            handler->callback(handler->cookie);
        }
    }
    vic_ack_irqs(irqs);
    return;
}

// ack vic should clear uarticr to acknoledge, else interupts wont be requested again
// should also ack at the vic level ?
//
void vic_ack_irqs(uint32_t irqs){
    if (irqs & UART0_IRQ_MASK) {
        // clear/ack the interrupt at the uart level
        mmio_write32(UART0, UART_ICR, 1 << RX_IRQ);
        mmio_write32(VIC_BASE_ADDR, VICINTCLEAR, UART0_IRQ_MASK);
    }
}

void * (uart_handler)(uint32_t uart,void* callback(char)) {
    kprintf ("Enter uart %d\n", uart);
    char c;
    if (uart_receive(uart,&c))
        callback(c);
    
}

/*
 * VIC behavior:
 */
// this is the equivalent of the init of the slide in week 3
void irqs_setup(){
    mmio_write32(VIC_BASE_ADDR, VICINTSELECT, 0);
    _irqs_setup();
}
// this is the equivalent of the core_enable_interrupts of the slide in week 3
void irqs_enable(){
    uart_irq_enable(UART0,RX_IRQ);
    uart_irq_enable(UART0,TX_IRQ);
    irq_enable(UART0_IRQ,(uart_handler)(UART0,console_echo),NULL);
    _irqs_enable();
}
// this is the equivalent of the core_disable_interrupts of the slide in week 3
void irqs_disable(){
    uart_irq_disable(UART0,RX_IRQ);
    uart_irq_disable(UART0,TX_IRQ);
    irq_disable(UART0_IRQ);
    _irqs_disable();
}
// this is the equivalent of the halt of the slide in week 3
void wfi(void){
    _wfi();
}

// for the following functions uart is in practice UART0 (void*) 0x101f1000

// enables uart to send interrupts
void uart_irq_enable(void* uart,uint32_t irq){
  // get the current mask
  uint32_t mask = mmio_read32(uart, UART_IMSC);
  // add our irq to it
  mask |= (1<<irq);
  mmio_write32(uart, UART_IMSC, mask);
}

// disables uart to send interrupts
void uart_irq_disable(uint32_t uart,uint32_t irq){
  // same as above, we assume the caller knows what they are doing
  uint32_t mask = mmio_read32(uart, UART_IMSC);
  // add our irq to it
  mask &= ~(1<<irq);
  mmio_write32(uart, UART_IMSC, mask);
}

/*
 * Enable the given interrupt,
 * like UART0_IRQ
 */
void irq_enable(uint32_t irq,void(*callback)(uint32_t,void*),void*cookie){
    uint32_t mask = (1<<irq);
    mmio_write32(VIC_BASE_ADDR, VICINTENABLE, mask);
    // set callback and cookie
    handlers[irq].callback = callback;
    handlers[irq].cookie = cookie;
    return;
}

/*
 * Disable the given interrupt,
 * like UART0_IRQ
 */
void irq_disable(uint32_t irq){
    uint32_t mask = (1<<irq);
    mmio_write32(VIC_BASE_ADDR, VICINTCLEAR, mask);
    // clear callback and cookie
    handlers[irq].callback = NULL;
    handlers[irq].cookie = NULL;
    return;
}

