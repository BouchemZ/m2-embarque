#include "console.h"

/*
 * Terminal Size: 
 *   - rows are horizontal 24
 *   - columns are vertical 80
*/

// current cursor position
typedef struct {
    uint16_t row;
    uint16_t col;
} cursor_t;

static cursor_t cursor;

// states for escape sequence  for the up, down, left, right arrows and more
typedef enum {
    IDLE,
    ESC,
    ESC_BRACKET,
    ESC_BRACKET_3,
} esc_state_t;

esc_state_t esc_state = IDLE;

char empty_line[80] = {0};

extern char line[80];
extern uint8_t offset;

extern uint8_t history_idx;

extern uint32_t irq_timer_tick;

void kprintf(const char *fmt, ...);

static size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}


// cursor loop
int  cursor_ind = 0;
char cursor_char[8]= { '|', '/', '-', '\\', '|', '/', '-', '\\', };

void blink_cursor(){
  uart_send_string(UART0,"\033[s");
  uart_send(UART0,cursor_char[cursor_ind]);
  uart_send_string(UART0,"\033[u");
  // re write the value where we are so cursor doesnt write over texts
  cursor_ind  = (cursor_ind == 7) ? 0 : cursor_ind + 1;
}

void da_vinci(char* s){
  char res[80];
  int len = strlen(s);
  for (int i=0; i<len; i++){
    res[i] = s[len-1-i];
  }
  res[len] = '\0';

  console_clear();
  cursor_at(3,1);
  kprintf("Input:\n");
  kprintf("%s\n",s);
  kprintf("Da Vinci says:\n");
  kprintf("%s\n", res);
  cursor_at(2,1);
}

void update_top_line(){
    //save cursor
    uart_send_string(UART0,"\033[s");
    
    cursor_at(1,1);
    kprintf(empty_line);
    cursor_at(1,1);
    kprintf("Time : %u",irq_timer_tick);
    //retunr to saved pos
    uart_send_string(UART0,"\033[u");
}

/*
 * Function to reset the line buffer, also sets offset to 0
 */
void reset_line_buffer(){
    for(int i=0; i<80; i++){
        line[i] = '\0';
    }
    offset = 0;
}

/*
 * Function to move the cursor left
 */
void cursor_left(){
    if(cursor.col>1){
        cursor.col -= 1;
        uart_send_string(UART0,"\033[1D");
    }
}

/*
 * Function to move the cursor right
 */
void cursor_right(){
    if(cursor.col<offset+1){
        cursor.col += 1;
        uart_send_string(UART0,"\033[1C");
    }
}

/*
 * Function to move the cursor down
 */
void cursor_down(){
    if(history_idx>0){
        history_idx--;
        copy_from_history(line);
        offset = strlen(line);
    }else{
        history_idx = -1;
        reset_line_buffer();
    }
    cursor_at(2,1);
    kprintf(empty_line);
    kprintf(history[history_idx]);
    /*
    if(cursor.row<24){
        cursor.row += 1;
        uart_send_string(UART0,"\033[1B");
    }
    */
}

/*
 * Function to move the cursor up
 */
void cursor_up(){
    if(history_idx < HISTORY_SIZE-1) history_idx ++;
    cursor_at(2,1);
    kprintf(empty_line);
    copy_from_history(line);
    offset = strlen(line);
    cursor_at(2,1);
    kprintf(line);
    /*
    if(cursor.row>0){
        cursor.row -= 1;
        uart_send_string(UART0,"\033[1A");
    }
    */
}

/*
 * Function to move the cursor to the given coordinates
 */ 
void cursor_at(int row, int col){
    cursor.row = row;
    cursor.col = col;
    kprintf("\033[%u;%uH", row, col);
}

/*
 * Functions to obtain the current cursor position 
 */
void cursor_position(int* row, int* col){
    *row = cursor.row;
    *col = cursor.col;
}

/* 
 * Functions to hide/show the terminal cursor
 */
void cursor_hide(){
    uart_send_string(UART0,"\033[?25l");
}
void cursor_show(){
    uart_send_string(UART0,"\033[?25h");
}

/*
 * Function to set the color, either for the ink or background
 */
void console_color(uint8_t color){
    uart_send_string(UART0,"\033[55m");
}

/*
 * Clears the terminal, like the bash command `clear`.
 * Positions the cursor at (0,0).
 */
void console_clear(){
    uart_send_string(UART0,"\033[H\033[2J");
}

void (*line_callback)(char*);

/*
 * Initializes the console, giving the callback
 * to call for each line entered on the keyboard.
 * A line is a C string but contains only ASCII 
 * characters ([32-126]), as a C string it is 
 * terminated by a '\0'.
 * A line is validated by the end user by hitting 
 * the key `Enter`.
 */
void console_init(void (*callback)(char*)){
    /* save the callback for later invocation */
    reset_line_buffer();
    line_callback = callback;
    cursor_hide();
    console_clear();
}

/*
 * Call this function with every byte read from the "keyboard".
 * Echoes to the terminal only ASCII characters ([32-126]).
 * Recognized special characters:
 *   - arrow keys (left,right,up,down)
 *   - delete key
 *   - backspace (code 127 or 8)
 *   - ctrl-c to clear the terminal
 */
void console_echo(uint8_t byte){

    switch (esc_state)
    {
    case IDLE:
        if(byte == 127 || byte == 8){
            if(offset == cursor.col -1){
                offset --;
                line[cursor.col-1] = '\0';
                cursor_left();
            }else{
                line[cursor.col-1] = ' ';
                cursor_left();

            }
        }
        // if break char we do the callback
        if( byte == '\r'){
            if (line_callback != NULL) {
                line_callback(line);
                add_to_history(line);
                history_idx = -1;
                reset_line_buffer();
                cursor_at(2,1);
            }
            return;
        }
        // ctrl + c
        if (byte == 3){
            reset_line_buffer();
            console_clear();
        }
        // backspace
        // if printable char
        if (byte>=32 && byte<=126){
            // if at end of line save position, print char, restore position
            if (cursor.col==78){
                uart_send_string(UART0,"\033[s");
                uart_send(UART0,byte);
                uart_send_string(UART0,"\033[u");
                //here offset should also be at the end of line
                line[offset] = byte;
            // else just print char and sync cursor
            }else{
                uart_send(UART0,byte);
                cursor.col += 1;
                line[offset] = byte;
                offset++;
            }
        }else if (byte == 27) {
            esc_state = ESC;
        }
        break;
    
    case ESC:
        if (byte == 91) {
            esc_state = ESC_BRACKET;
        } else {
            esc_state = IDLE;
        }
        break;
    case ESC_BRACKET:
        switch (byte)
        {
            case 65:
                cursor_up();
                esc_state = IDLE;
                break;
            case 66:
                cursor_down();
                esc_state = IDLE;
                break;
            case 67:
                cursor_right();
                esc_state = IDLE;
                break;
            case 68:
                cursor_left();
                esc_state = IDLE;
                break;
            case 51:
                // this is for the delete escape sequence
                esc_state = ESC_BRACKET_3;
                break;
            default:
                esc_state = IDLE;
        }
        break;
    case ESC_BRACKET_3:
        /* expecting '~' */
        if (byte == 126){
            uart_send_string(UART0,"\033[s");
            uart_send(UART0,' ');
            uart_send_string(UART0,"\033[u");
        }
        esc_state = IDLE;
        break;
    }
}