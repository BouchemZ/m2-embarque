#include "timer.h"

// inits the time
// it enables the module with periodic mode and the interrupts
// with a period of 1ms
void timer_init(void* timer){
    // finish periodiic
    uint32_t value = TIMER_ENABLE | TIMER_MODE_PERIODIC | TIMER_SIZE_32;
    mmio_write32(timer, TIMER_LOAD, 1000); // CPU fixed at 1MHz so is 1ms period
    mmio_write32(timer, TIMER_CONTROL, value);
}

//enable the timer interrupts
void timer_irq_enable(void* timer){
    uint32_t value = mmio_read32(timer, TIMER_CONTROL);
    value |= TIMER_IRQ_ENABLE;
    mmio_write32(timer, TIMER_CONTROL, value);
}

//disable the timer interrupts
void timer_irq_disable(void* timer){
    uint32_t value = mmio_read32(timer, TIMER_CONTROL);
    value &= ~TIMER_IRQ_ENABLE;
    mmio_write32(timer, TIMER_CONTROL, value);
}

//clear the timer interrupts
void timer_clear_interrupt(void* timer){
    mmio_write32(timer, TIMER_INTCLR, 0x1);
}