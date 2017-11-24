#ifndef FILE_OPERATION_H
#define FILE_OPERATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

#define size_path 1<<10

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

void tail(char** args)
{
	printf("tail \n");
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
		printf("%d \n", status_pid);
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
