#include "history.h"

// new_line is char[80]
void add_to_history(char* new_line){
    for(int j=HISTORY_SIZE-1;j>0;j--){
        // copy from top to bottom
        for(int i=0;i<80;i++){
            history[j][i] = history[j-1][i];
        }
    }
    //copy new_line at history[0] most recent
    for(int i=0;i<80;i++){
        history[0][i] = new_line[i];
    }
}

// destination line
void copy_from_history(char* dest_line){
    for(int i=0;i<80;i++){
        dest_line[i] = history[history_idx][i];
    }
}