#ifndef FILE_OPERATION_H
#define FILE_OPERATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <ctype.h>

#define size_path 1<<10
#define size_command 300
#define size_read 1<<12
#define size_history 5000

typedef struct{ char* file_name;
				int nr; // to display
				int off_set;
			} file_tail;

void clean_args(char** args);

int atoi_2(char * x);

char test_path(char* path);

/* rt value
 * 1-is existing file
 * 2-is folder
 * 3-does not exist
 */
char test_if_file(char *file);

void init_path(char *path, char** curr_path);

void display_history(char** args, char** history, int* pos_history);

void change_directory(char** curr_path,char** args);

void* find_offset_file_tail_row(void* arg_struct);

void* find_offset_file_tail_byte(void* arg_struct);

void display_from_file_with_offset(char* file, int offset);

void solve_tail_func(char need_header_title, char type_line_or_bits, file_tail **list_files, int size_list);

void tail(char** args, char* path_process_base);

int stricmp(const char* s1,const char* s2);

void* uniq_thread_duplicate_sensitive(void *file_arg);

void* uniq_thread_duplicate_non_sensitive(void *file_arg);

void* uniq_thread_default_sensitive(void *file_arg);

void* uniq_thread_default_non_sensitive(void *file_arg);

void* uniq_thread_uniq_sensitive(void *file_arg); /*care trebe schimbat */

void* uniq_thread_uniq_non_sensitive(void *file_arg); /*care trebe schimbat */

void uniq(char **args, char* path_process_base);

int process_command(char** curr_path, char** args, char** history, int* pos_history, int input, int first, int last, char* path_process_base, char* input_file_redirect , char *output_file_redirect);

#endif
