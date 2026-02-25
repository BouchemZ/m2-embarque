#include "console.h"

/*
 * Terminal Size: 
 *   - rows are horizontal 24
 *   - columns are vertical 79
*/

// current cursor position
typedef struct {
    uint16_t row;
    uint16_t col;
} cursor_t;

static cursor_t cursor = {0, 0};

// states for escape sequence  for the up, down, left, right arrows and more
typedef enum {
    IDLE,
    ESC,
    ESC_BRACKET,
    ESC_BRACKET_3,
} esc_state_t;

esc_state_t esc_state = IDLE;

extern char line[79];
extern uint8_t offset;

/*
 * Function to reset the line buffer, also sets offset to 0
 */
void reset_line_buffer(){
    for(int i=0; i<79; i++){
        line[i] = '\0';
    }
    offset = 0;
}

/*
 * Function to move the cursor left
 */
void cursor_left(){
    if(cursor.col>0){
        cursor.col -= 1;
        offset -=1;
        uart_send_string(UART0,"\033[1D");
    }
}

/*
 * Function to move the cursor right
 */
void cursor_right(){
    if(cursor.col<79){
        line[offset] = ' ';
        offset++;
        cursor.col += 1;
        uart_send_string(UART0,"\033[1C");
    }
}

/*
 * Function to move the cursor down
 */
void cursor_down(){
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
    cursor_at(0,0);
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
    // get pos instead of playing directly with the cursor
    int col;
    int row;
    cursor_position(&row,&col);

    switch (esc_state)
    {
    case IDLE:
        // if break char we do the callback
        if( byte == '\r'){
            if (line_callback != NULL) {
                line_callback(line);
                reset_line_buffer();
                cursor_at(row+1,0);
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
            if (col==79){
                uart_send_string(UART0,"\033[s");
                uart_send(UART0,byte);
                uart_send_string(UART0,"\033[u");
                //here offset should also be at the end of line
                line[offset] = byte;
            // else just print char and sync cursor
            }else{
                uart_send(UART0,byte);
                line[offset] = byte;
                offset++;
                cursor_at(row,col+1);
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