#include "main.h"
#include "console.h"
#include "uart.h"

/*
 * Define ECHO_ZZZ to have a periodic reminder that this code is polling
 * the UART, actively. This means the processor is running continuously.
 * Polling is of course not the way to go, the processor should halt in
 * a low-power state and wake-up only to handle an interrupt from the UART.
 * But this would require setting up interrupts...
 */
#define ECHO_ZZZ

void panic() {
  while (1)
	  ;
}

// faire une boucle de 1sec
void wait(){
	for (int i=0; i<1000000; i++){
	}
}

uint32_t stack_top;

void check_memory() {
  void *max = (void*)MEMORY;
  void *addr = &stack_top;
  if (addr >= max)
    panic();
}

// cursor loop
int  cursor_ind = 0;
char cursor_char[8]= { '|', '/', '-', '\\', '|', '/', '-', '\\', };

void blink_cursor(){
  uart_send_string(UART0,"\033[s");
  uart_send(UART0,cursor_char[cursor_ind]);
  uart_send_string(UART0,"\033[u");
  cursor_ind  = (cursor_ind == 7) ? 0 : cursor_ind + 1;
}

void does_nothing(){
  return;
}
/**
 * This is the C entry point, upcalled once the hardware has been setup properly
 * in assembly language, see the startup.s file.
 */
void _start() {
  check_memory();
  console_init(does_nothing);
  int counter = 0;
  while (1) {
    counter ++;

    if (counter == 500000) {
      blink_cursor();
      counter = 0;
    }

    uint8_t c;
    if (0==uart_receive(UART0,&c))
      continue;
    console_echo(c);
  }
}



