#ifndef FILE_OPERATION_H
#define FILE_OPERATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

#define size_path 300

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

void init_path(char *path, char** curr_path)
{
	if(test_path(path)){
		*curr_path=(char *)malloc(sizeof(char) * size_path);
		if(*curr_path == NULL){
			printf("Fail tp alloc mam for FILE PATH \n");
			exit(2);
		}
	
		strcpy(*curr_path,path);
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
	{
		strcpy(*curr_path,"/home");
		chdir(*curr_path);
	}
	else
	{
		char* temp_args=args[1];
		
		if(strstr(temp_args,"/home") == temp_args)
		{
			if(test_path(temp_args))
			{
				chdir(temp_args);
				strcpy(*curr_path,temp_args);
			}
			else
				printf("shell_GABI: cd: %s: No such file or directory \n",temp_args);
		}
		else
		{
			temp_args[strlen(temp_args)]=0;
			printf("%s \n",temp_args);
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
			
			//~ strcat(temp_curr_path,temp_args);
			printf("%s \n",temp_curr_path);
			
			if(test_path(temp_curr_path))
			{
				strcpy(*curr_path,temp_curr_path);
				chdir(*curr_path);
			}
			else
			{
				printf("shell_GABI: cd: %s: No such file or directory \n",*curr_path);
			}
		}
	}
}

void process_command(char** curr_path, char** args, char** history, int* pos_history)
{
	if(strcmp(args[0],"exit") == 0)
		exit(0);
	if(strcmp(args[0],"history") == 0)
		display_history(args,history,pos_history);
	if(strcmp(args[0],"cd") == 0)
		change_directory(curr_path,args);
	else{
	
	pid_t id=fork();
	int status_pid=-1;
	
	if(id == 0){
		chdir(*curr_path);
		if(execvp(args[0],args) < 0)
			exit(1);
		exit(0);
	}
	else
		waitpid(id,&status_pid,0);
	
	printf("pid status %d \n",status_pid);
	}
	
	clean_args(args);
	
}





#endif
