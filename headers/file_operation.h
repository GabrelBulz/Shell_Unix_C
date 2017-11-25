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

#define size_path 1<<10
#define size_command 300
#define size_read 1<<12

typedef struct{ char* file_name;
				int nr; // to display
				int off_set;
			} file_tail;

void clean_args(char** args)
{
	int x=0;
	
	while(args[x] != NULL)
	{
		free(args[x]);
		x++;
	}
	
	free(args);
}

int atoi_2(char * x)
{
	if(strlen(x) > 12) /* cannot store this much */
		return -1;
	else
	{
		int temp=0;
		int cont=0;
		
		while(x[cont] != 0){
			if(x[cont] < 48 || x[cont] > 57) /* daca nu e cifra */
				return -2;
			temp=temp*10+(x[cont]-48);
			cont++;
		}
		
		return temp;
	}
}


char test_path(char* path)
{
	char test=0;
	DIR* dir = opendir(path);
	
	if (dir)
	{
		test=1;
		closedir(dir);
	}
	
	return test;
}

/* rt value
 * 1-is existing file
 * 2-is folder
 * 3-does not exist
 */
char test_if_file(char *file)
{
	if(test_path(file))
		return 2;
	int id=open(file,O_RDONLY);
	if(id < 0)
    {
		return 3;
	}
    else
    {
		close(id);
		return 1;
	}
 }
	

void init_path(char *path, char** curr_path)
{
	if(test_path(path)){
		*curr_path=(char *)malloc(sizeof(char) * size_path);
		if(*curr_path == NULL){
			printf("Fail tp alloc mam for FILE PATH \n");
			exit(2);
		}
	
		strcpy(*curr_path,path);
		chdir(*curr_path);
	}
	else
		printf("Invalid path \n");
}

void display_history(char** args, char** history, int* pos_history)
{
	if(args[1] == NULL)
	{
		int cont=0;
		for(cont=0; cont<*pos_history; cont++)
			printf("    %d %s \n",cont,history[cont]);
	}
	else
	{
		int to_display=atoi_2(args[1]);
		
		if(to_display == -1)
			printf(" Too large number for history \n");
		if(to_display == -2)
			printf("shell_GABI: history: %s: numeric argument required \n",args[1]);
		if(to_display > 0)
		{
			if(to_display > *pos_history-1)
				to_display= *pos_history;
				
			int cont=*pos_history-to_display;
			
			while(cont < *pos_history){
				printf("    %d %s \n",cont,history[cont]);
				cont++;
			}
		}
	}		
}

void change_directory(char** curr_path,char** args)
{
	if(args[1] == NULL)   /* case cd with no arguments */
		chdir("/home");
	
	else
	{
		char* temp_args=args[1];
					
		if(strstr(temp_args,"/home") == temp_args)
		{
			if(test_path(temp_args))
				chdir(temp_args);
			else
				printf("shell_GABI: cd: %s: No such file or directory \n",temp_args);
		}
		else
		{
			char* temp_curr_path=(char *)malloc(sizeof(char) * size_path);
			if(temp_curr_path == NULL){
				printf("Fail to alloc mem in change directory \n");
				exit(2);
			}
			
			strcpy(temp_curr_path,*curr_path);
			
			if(temp_args[0] != '/')
				strcat(temp_curr_path,"/");
				
			if(strlen(temp_args)+strlen(temp_curr_path) > size_path)
			{
				char* aux=(char *)malloc(sizeof(char)*2*size_path);
				if(aux == NULL){
					printf("Fail to alloc mem in change dir \n");
					exit(2);
				}
				strcpy(aux,temp_curr_path);
				
				free(temp_curr_path);
				temp_curr_path=aux;
			}
			
			strcat(temp_curr_path,temp_args);
			
			if(test_path(temp_curr_path))
				chdir(temp_curr_path);
			else
			{
				printf("shell_GABI: cd: %s: No such file or directory \n",temp_curr_path);
			}
		}
	}

	char* temp_cwd=(char*)malloc(sizeof(char) * size_path);
	if(temp_cwd == NULL){
		printf("Fail to alloc mem in change dir cwd \n");
		exit(2);
	}
	

	char temp_pwd[size_path];
    getcwd(temp_pwd,sizeof(temp_pwd));
    strcpy(*curr_path,temp_pwd);
}

void* find_offset_file_tail_row(void* arg_struct)
{
	file_tail **file_struct=(file_tail **)arg_struct;
	
	int test=test_if_file((*file_struct)->file_name);
	if(test != 1)
		(*file_struct)->off_set=test*(-1);
	else
	{
		int file_id=open((*file_struct)->file_name,O_RDONLY);
		if(file_id < 0)
        {
			(*file_struct)->off_set=-4;   /* couldn't open file */
			pthread_exit(0);
		}
		
		/* set file fointer to end of file */
		off_t pointer=lseek(file_id,0L,SEEK_END);


		int cont_line=0;
		char buff;
		
		while(cont_line <= (*file_struct)->nr && pointer > lseek(file_id,0L,SEEK_SET))
		{
			pointer--;
			lseek(file_id,pointer,SEEK_SET);
			
			read(file_id,&buff,1);
			if(buff == '\n')
			cont_line++;
		}
		
		(*file_struct)->off_set=(int)pointer;
		close(file_id);
		pthread_exit(0);	
	}
	pthread_exit(0);
}

void* find_offset_file_tail_byte(void* arg_struct)
{
	file_tail **file_struct=(file_tail **)arg_struct;
	
	int test=test_if_file((*file_struct)->file_name);
	if(test != 1)
		(*file_struct)->off_set=test*(-1);
	else
	{
		int file_id=open((*file_struct)->file_name,O_RDONLY);
		if(file_id < 0)
        {
			(*file_struct)->off_set=-4;   /* couldn't open file */
			pthread_exit(0);
		}
		
		/* set file fointer to end of file */
		off_t pointer=lseek(file_id,0L,SEEK_END);
		if(pointer-(off_t)(*file_struct)->nr > lseek(file_id,0L,SEEK_SET))
		{
			(*file_struct)->off_set=(int)(pointer-(off_t)(*file_struct)->nr);
			close(file_id);
			pthread_exit(0);
		}
		else
		{
			(*file_struct)->off_set=(int)lseek(file_id,0L,SEEK_SET);
			close(file_id);
			pthread_exit(0);
		}	
	}
	pthread_exit(0);
}

void display_from_file_with_offset(char* file, int offset)
{
	int id=open(file,O_RDONLY);
	
	if(id < 0)
	{
		printf("cannot open file %s \n",file);
		return;
	}
	
	lseek(id,offset,SEEK_SET);
	
	char *buff=(char*)malloc(sizeof(char) * size_read);
	if(buff == NULL)
	{
		printf("Fail to alloc mem in display_from_file_with_offset \n");
		return;
	}
	
	while(read(id,buff,size_read) > 0)
		printf("%s",buff);
		
	close(id);
}

void solve_tail_func(char need_header_title, char type_line_or_bits, file_tail **list_files, int size_list)
{
	///create a list of threads
	pthread_t list_threads[size_list];
	
	int i;
	
	if(type_line_or_bits == 1)
		for(i=0; i<size_list; i++)
			pthread_create(&list_threads[i],NULL,find_offset_file_tail_byte,&list_files[i]);
	else
		for(i=0; i<size_list; i++)
			pthread_create(&list_threads[i],NULL,find_offset_file_tail_row,&list_files[i]);
		
	///wait for threads
	for(i=0; i<size_list; i++)
		pthread_join(list_threads[i],NULL);
	
	if(size_list > 1 && need_header_title == -1)
		need_header_title=1;
		
	for(i=0; i<size_list; i++)
	{
		if(need_header_title == 1)
			printf("===> %s <=== \n",list_files[i]->file_name);
			
		switch(list_files[i]->off_set)
		{
			case -2:
			{
				printf("tail: error reading '%s': Is a directory \n",list_files[i]->file_name);
				break;
			}
			case -3:
			{
				printf("tail: cannot open '%s' for reading: No such file or directory \n",list_files[i]->file_name);
				break;
			}
			default:
			{
				display_from_file_with_offset(list_files[i]->file_name, list_files[i]->off_set);
				break;
			}
		}
	}
}

void tail(char** args)
{
	if(args[1] == NULL)
		printf("shell_GABI: Must provide a file \n");
	else
	{
		char need_header_title=-1;
		char type_line_or_bits=0; /* if 0-then it'll take nr of lines 1-Nr of bits */
		long int nr_to_display=10;
		
		file_tail **list_files=(file_tail**)malloc(sizeof(file_tail*) * size_command);
		if(list_files == NULL)
		{
			printf("Fail to alloc mem for Files List in tail");
			exit(2);
		}
		
		int cont_list_file=0;
		int cont=1;
		while(args[cont] != NULL)
		{
			if(strcmp(args[cont],"-c") == 0)
			{
				cont++;
				if(args[cont] == NULL){
					printf("tail: option requires an argument -- 'c' \n");
					break;
				}
				
				long int nr=atoi_2(args[cont]);
				
				if(nr == -1){
					printf("tail: invalid number of bytes: %s: Value too large for defined data type \n",args[cont]);
					break;
				}
				if(nr == -2){
					printf("tail: %s: invalid nr of bytes \n",args[cont]);
					break;
				}
				
				type_line_or_bits=1; /* set for bits */
				nr_to_display=nr;
				
				cont++;
				continue;
			}
			
			if(strcmp(args[cont],"-n") == 0)
			{
				cont++;
				if(args[cont] == NULL){
					printf("tail: option requires an argument -- 'n' \n");
					break;
				}
				
				long int nr=atoi_2(args[cont]);
				
				if(nr == -1){
					printf("tail: invalid number of lines: %s: Value too large for defined data type \n",args[cont]);
					break;
				}
				if(nr == -2){
					printf("tail: %s: invalid nr of lines \n",args[cont]);
					break;
				}
				
				type_line_or_bits=0; /* set for lines */
				nr_to_display=nr;
				
				cont++;
				continue;
			}
			if(strcmp(args[cont],"-q") == 0)
			{
				need_header_title=0; /* don't display headers */
				cont++;
				continue;
			}
			if(strcmp(args[cont],"-v") == 0)
			{
				need_header_title=1; /*display headers */
				cont++;
				continue;
			}
			
			/* store files name */
			list_files[cont_list_file]=(file_tail*)malloc(sizeof(file_tail));
			if(list_files[cont_list_file] == NULL){
				printf("Fail to alloc mem for list_files in while \n");
				exit(2);
			}
			
			list_files[cont_list_file]->file_name=(char*)malloc(sizeof(char) * strlen(args[cont]) + 1);
			if(list_files[cont_list_file]->file_name == NULL){
				printf("Fail to alloc mem for list_files in while \n");
				exit(2);
			}
			strcpy(list_files[cont_list_file++]->file_name,args[cont++]);
		}
		
		list_files[cont_list_file]=NULL;
		
		/* set nr for each struct */
		int i;
		for(i=0; i<cont_list_file; i++)
			list_files[i]->nr=nr_to_display;
		/* set all offset to 0*/
		for(i=0; i<cont_list_file; i++)
			list_files[i]->off_set=0;
			
		solve_tail_func(need_header_title,type_line_or_bits,list_files,cont_list_file);
	}
	return;
}




void process_command(char** curr_path, char** args, char** history, int* pos_history)
{
	if(strcmp(args[0],"exit") == 0)
		exit(0);
		
	int pip[2];
	if(pipe(pip) < 0)
	{
		printf("Fail to create pipe in process_command \n");
		exit(5);
	}
	
	int status_pid=-1;
	int id=fork();
	
	if(id < 0)
	{
		printf("Fork failed \n");
		exit(4);
	}
	
	if(id == 0){ /* child */
		
		if(strcmp(args[0],"history") == 0)
		{
			close(pip[1]);
			close(pip[0]);
			display_history(args,history,pos_history);
			exit(0);
		}
		if(strcmp(args[0],"tail") == 0)
		{
			close(pip[1]);
			close(pip[0]);
			tail(args);
			exit(0);
		}
		if(strcmp(args[0],"cd") == 0)
		{
			close(pip[0]);
			change_directory(curr_path,args);
			
			/* get curr path after execution of cd */
			char temp_pwd[size_path];
			getcwd(temp_pwd,sizeof(temp_pwd));
			
			write(pip[1],temp_pwd,size_path*sizeof(char));
			close(pip[1]);
			
			exit(0);
		}
		else
		{
			close(pip[1]);
			close(pip[0]);
			chdir(*curr_path);
			if(execvp(args[0],args) < 0)
				exit(-1);
			exit(0);
		}
	}
	else{ /* parent */
		
		close(pip[1]); /* pip1 to write in*/
					   /* pip0 to read from*/
					   
		char temp_path[size_path];
		if(read(pip[0],temp_path,size_path) > 0)
		{
			strcpy(*curr_path,temp_path);
			chdir(*curr_path);
		}
		waitpid(id,&status_pid,0);
			clean_args(args);
		close(pip[0]);
		
		if(status_pid != 0)
		{
			int cont=0;
			while(args[cont] != NULL)
				printf("%s ",args[cont++]);
				
			printf(":command not found \n");
		}
	}
		
		
		
		
	//~ if(strcmp(args[0],"history") == 0)
	//~ {
		//~ display_history(args,history,pos_history);
		//~ return;
	//~ }
	//~ if(strcmp(args[0],"tail") == 0)
	//~ {
		//~ tail(args);
		//~ return;
	//~ }
	//~ if(strcmp(args[0],"cd") == 0)
	//~ {
		//~ change_directory(curr_path,args);
		//~ return;
	//~ }
	//~ else{
	
		//~ pid_t id=fork();
		//~ int status_pid=-1;
	
		//~ if(id == 0){
			//~ chdir(*curr_path);
			//~ if(execvp(args[0],args) < 0)
				//~ exit(1);
			//~ exit(0);
		//~ }
		//~ else
		//~ {
			//~ waitpid(id,&status_pid,0);
			//~ clean_args(args);
		//~ }
	
		//~ printf("pid status %d \n",status_pid);
	//~ }
	
}


#endif
