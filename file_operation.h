#ifndef FILE_OPERATION_H
#define FILE_OPERATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define size_path 300

void init_path(char *path, char * curr_path)
{
	curr_path=(char *)malloc(sizeof(char) * size_path);
	if(curr_path == NULL){
		printf("Fail tp alloc mam for FILE PATH \n");
		exit(2);
	}
	
	strcpy(curr_path,path);
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



#endif
