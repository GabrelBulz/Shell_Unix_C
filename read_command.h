#ifndef READ_COMMAND_H
#define READ_COMMAND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define size_history 5000
#define size_command 300

#define cursorbackward(x) printf("\033[%dD", (x))
#define cursorforward(x) printf("\033[%dC", (x))
#define difference pos_buff-pos_buff_left_right


/** ascii has recmap some keys from their nomal value to another value
 * backspace = 127 instead of 8
 * up key \330A
 * down key \330B
 */

void clear_history_if_full (int* pos, char** hist);

void inset_to_history(int *pos, char** history, char* to_insert);

char** parse_command(char* buff_command);

char* get_line_command_case_up(int* pos_history, char ** history, char* curr_path);

char* get_line_command(int* pos_history, char ** history, char* curr_path);

int main_read_command();


#endif
