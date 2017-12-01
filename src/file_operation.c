#include "../headers/read_command.h"
#include "../headers/file_operation.h"

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

void tail(char** args, char* path_process_base)
{
	if(args[1] == NULL)
	{
		char *path_file_buffer="TempFileTailx01.txt";
		int TempBuffFile = open(path_file_buffer, O_RDWR|O_CREAT|O_APPEND, 0600);
		
		if(TempBuffFile < 0)
		{
			printf("A error has occur durring the creating the buffer file in TAIL \n");
			exit(7);
		}
		
		char *temp_buff=(char*)malloc(sizeof(char) * (size_read) + 1);
		if(temp_buff == NULL){
			printf("Fail to alloc mem for buff in TAIL \n");
			exit(2);
		}
		
		int read_size;
		while((read_size=read(0,temp_buff,size_read)))
			write(TempBuffFile,temp_buff,read_size);
			
		close(TempBuffFile);
		
		file_tail **list_files=(file_tail**)malloc(sizeof(file_tail*) * size_command);
		if(list_files == NULL)
		{
			printf("Fail to alloc mem for Files List in tail");
			exit(2);
		}
		
		int cont_list_file=0;
		
		list_files[cont_list_file]=(file_tail*)malloc(sizeof(file_tail));
			if(list_files[cont_list_file] == NULL){
				printf("Fail to alloc mem for list_files in while \n");
				exit(2);
			}
			
			list_files[cont_list_file]->file_name=(char*)malloc(sizeof(char) * strlen(path_file_buffer) + 1);
			if(list_files[cont_list_file]->file_name == NULL){
				printf("Fail to alloc mem for list_files in while \n");
				exit(2);
			}
			strcpy(list_files[cont_list_file++]->file_name,path_file_buffer);
			
			list_files[cont_list_file]=NULL;
			
			list_files[0]->nr=10;
			
			solve_tail_func(0,0,list_files,cont_list_file);
			
			remove(path_file_buffer);
	}
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
			
			
		if(cont_list_file > 0)	
			solve_tail_func(need_header_title,type_line_or_bits,list_files,cont_list_file);
		else
		{
			char *path_file_buffer="TempFileTailx01.txt";
			int TempBuffFile = open(path_file_buffer, O_RDWR|O_CREAT|O_APPEND, 0600);
		
			if(TempBuffFile < 0)
			{
				printf("A error has occur durring the creating the buffer file in TAIL \n");
				exit(7);
			}
		
			char *temp_buff=(char*)malloc(sizeof(char) * (size_read) + 1);
			if(temp_buff == NULL){
				printf("Fail to alloc mem for buff in TAIL \n");
				exit(2);
			}
		
			int read_size;
			while((read_size=read(0,temp_buff,size_read)))
				write(TempBuffFile,temp_buff,read_size);
			
			close(TempBuffFile);

		
			list_files[cont_list_file]=(file_tail*)malloc(sizeof(file_tail));
				if(list_files[cont_list_file] == NULL){
					printf("Fail to alloc mem for list_files in while \n");
					exit(2);
				}
			
				list_files[cont_list_file]->file_name=(char*)malloc(sizeof(char) * strlen(path_file_buffer) + 1);
				if(list_files[cont_list_file]->file_name == NULL){
					printf("Fail to alloc mem for list_files in while \n");
					exit(2);
				}
				strcpy(list_files[cont_list_file++]->file_name,path_file_buffer);
			
				list_files[cont_list_file]=NULL;
			
				list_files[0]->nr=nr_to_display;
				list_files[0]->off_set=0;
			
				solve_tail_func(0,type_line_or_bits,list_files,cont_list_file);
			
				remove(path_file_buffer);	
		}
	}
	return;
}

int stricmp(const char* s1,const char* s2)
{
	if(strlen(s1) != strlen(s2))
		return -1;
		
	int i=0;
	for(i=0; i<strlen(s1); i++)
	{
		int res=tolower(s1[i])-tolower(s2[i]);
		
		if(res != 0)
			return -1;
	}
	
	return 0;
}

void* uniq_thread_duplicate_sensitive(void *file_arg)
{
	char* file=(char *)file_arg;
	
	int test=test_if_file(file);
	if(test == 3){
		printf("uniq: %s: No such file or directory \n",file);
		pthread_exit(0);
	}
	if(test == 2){
		printf("uniq: %s: Is a directory directory",file);
		pthread_exit(0);
	}
	
	int id=open(file,O_RDONLY);
	if(id < 0){
		printf("Fail to open file in UNIQ_THREAD_DUPLICATE \n");
		pthread_exit(0);
	}

	int cont_temp_row=0;
	
	char* temp_row=(char*)malloc(sizeof(char) * size_read);
	if(temp_row == NULL){
		printf("Fail to alloc mem for temp_row in UNIQ_THREAD_DUP \n");
		pthread_exit(0);
	}
	
	char buff;
	char* prev_row=(char *)calloc(sizeof(char),2);
	int cont_temp_row_resized=1;
	int already_printed_once=0;
	
	while(read(id,&buff,1))
	{
		if(cont_temp_row == (size_read) -1)
		{
			temp_row[cont_temp_row]=0;
			char* aux=temp_row;
			
			cont_temp_row_resized++;
			
			temp_row=(char*)malloc(sizeof(char) * size_read * cont_temp_row_resized);
			if(temp_row == NULL){
				printf("Fail to alloc mem for temp_row in UNIQ_THREAD_DUP \n");
				pthread_exit(0);
			}
			
			strcpy(temp_row,aux);
			free(aux);
			
			cont_temp_row++;
		}
		
		if(buff == '\n' || buff == EOF)
		{
			temp_row[cont_temp_row]=0;
			
			/* case \n repeats */
			if(strlen(temp_row) < 1 && strlen(prev_row) < 1 && already_printed_once == 0)
			{
				printf("\n");
				already_printed_once=1;
			}
			else
			{
				if(prev_row != NULL)
				{
					if(strcmp(temp_row,prev_row) == 0 && already_printed_once == 0)
					{			
						printf("%s \n",temp_row);
						already_printed_once=1;
					}
					
					if(strcmp(prev_row,temp_row) != 0)
						already_printed_once=0;
				}
				
				free(prev_row);
				prev_row=temp_row;
			
				cont_temp_row=0;
				cont_temp_row_resized=1;

				temp_row=(char*)malloc(sizeof(char) * size_read * cont_temp_row_resized);
				if(temp_row == NULL){
					printf("Fail to alloc mem for temp_row in UNIQ_THREAD_DUP \n");
					pthread_exit(0);
				}
			}
			
		}
		else
			temp_row[cont_temp_row++]=buff;
		
	}
	
	free(prev_row);
	free(temp_row);
	
	close(id);
	
	pthread_exit(0);
}

void* uniq_thread_duplicate_non_sensitive(void *file_arg)
{
	char* file=(char *)file_arg;
	
	int test=test_if_file(file);
	if(test == 3){
		printf("uniq: %s: No such file or directory \n",file);
		pthread_exit(0);
	}
	if(test == 2){
		printf("uniq: %s: Is a directory directory",file);
		pthread_exit(0);
	}
	
	int id=open(file,O_RDONLY);
	if(id < 0){
		printf("Fail to open file in UNIQ_THREAD_DUP_NON_SENS \n");
		pthread_exit(0);
	}

	int cont_temp_row=0;
	
	char* temp_row=(char*)malloc(sizeof(char) * size_read);
	if(temp_row == NULL){
		printf("Fail to alloc mem for temp_row in UNIQ_THREAD_DUP_NON_SENS \n");
		pthread_exit(0);
	}
	
	char buff;
	char* prev_row=(char *)calloc(sizeof(char),2);
	int cont_temp_row_resized=1;
	int already_printed_once=0;
	
	while(read(id,&buff,1))
	{
		if(cont_temp_row == (size_read) -1)
		{
			temp_row[cont_temp_row]=0;
			char* aux=temp_row;
			
			cont_temp_row_resized++;
			
			temp_row=(char*)malloc(sizeof(char) * size_read * cont_temp_row_resized);
			if(temp_row == NULL){
				printf("Fail to alloc mem for temp_row in UNIQ_THREAD_DUP_NON_SENS \n");
				pthread_exit(0);
			}
			
			strcpy(temp_row,aux);
			free(aux);
			
			cont_temp_row++;
		}
		
		if(buff == '\n' || buff == EOF)
		{
			temp_row[cont_temp_row]=0;
			
			/* case \n repeats */
			if(strlen(temp_row) < 1 && strlen(prev_row) < 1 && already_printed_once == 0)
			{
				printf("\n");
				already_printed_once=1;
			}
			else
			{
				if(prev_row != NULL)
				{
					if(stricmp(temp_row,prev_row) == 0 && already_printed_once == 0)
					{			
						printf("%s \n",temp_row);
						already_printed_once=1;
					}
					
					if(stricmp(prev_row,temp_row) != 0)
						already_printed_once=0;
				}
				
				free(prev_row);
				prev_row=temp_row;
			
				cont_temp_row=0;
				cont_temp_row_resized=1;

				temp_row=(char*)malloc(sizeof(char) * size_read * cont_temp_row_resized);
				if(temp_row == NULL){
					printf("Fail to alloc mem for temp_row in UNIQ_THREAD_DUP_NON_SENS \n");
					pthread_exit(0);
				}
			}
			
		}
		else
			temp_row[cont_temp_row++]=buff;
		
	}
	
	free(prev_row);
	free(temp_row);
	
	close(id);
	
	pthread_exit(0);
}

void* uniq_thread_default_sensitive(void *file_arg)
{
	char* file=(char *)file_arg;
	
	
	
	int test=test_if_file(file);
	if(test == 3){
		printf("uniq: %s: No such file or directory \n",file);
		pthread_exit(0);
	}
	if(test == 2){
		printf("uniq: %s: Is a directory directory",file);
		pthread_exit(0);
	}
	
	int id=open(file,O_RDONLY);
	if(id < 0){
		printf("Fail to open file in UNIQ_THREAD_UNIQ \n");
		pthread_exit(0);
	}

	int cont_temp_row=0;
	
	char* temp_row=(char*)malloc(sizeof(char) * size_read);
	if(temp_row == NULL){
		printf("Fail to alloc mem for temp_row in UNIQ_THREAD_UNIQ \n");
		pthread_exit(0);
	}
	
	char buff;
	char* prev_row=(char *)calloc(sizeof(char),2);
	int cont_temp_row_resized=1;
	
	while(read(id,&buff,1))
	{
		if(cont_temp_row == (size_read) -1)
		{
			temp_row[cont_temp_row]=0;
			char* aux=temp_row;
			
			cont_temp_row_resized++;
			
			temp_row=(char*)malloc(sizeof(char) * size_read * cont_temp_row_resized);
			if(temp_row == NULL){
				printf("Fail to alloc mem for temp_row in UNIQ_THREAD_UNIQ \n");
				pthread_exit(0);
			}
			
			strcpy(temp_row,aux);
			free(aux);
			
			cont_temp_row++;
		}
		
		if(buff == '\n' || buff == EOF)
		{
			temp_row[cont_temp_row]=0;
		
			/* if cur row if different from the above one */
			{
				if(prev_row != NULL)
				{
					if(strcmp(temp_row,prev_row) != 0)
						printf("%s \n",temp_row);
				}
				else
					printf("%s\n",temp_row);
				
				free(prev_row);
				prev_row=temp_row;
			
				cont_temp_row=0;
				cont_temp_row_resized=1;

				temp_row=(char*)malloc(sizeof(char) * size_read * cont_temp_row_resized);
				if(temp_row == NULL){
					printf("Fail to alloc mem for temp_row in UNIQ_THREAD_UNIQ \n");
					pthread_exit(0);
				}
			}
			
		}
		else
			temp_row[cont_temp_row++]=buff;
		
	}
	
	free(prev_row);
	free(temp_row);
	
	close(id);
	
	pthread_exit(0);
}

void* uniq_thread_default_non_sensitive(void *file_arg)
{
	char* file=(char *)file_arg;
	
	
	
	int test=test_if_file(file);
	if(test == 3){
		printf("uniq: %s: No such file or directory \n",file);
		pthread_exit(0);
	}
	if(test == 2){
		printf("uniq: %s: Is a directory directory",file);
		pthread_exit(0);
	}
	
	int id=open(file,O_RDONLY);
	if(id < 0){
		printf("Fail to open file in UNIQ_THREAD_UNIQ_NON_SENS \n");
		pthread_exit(0);
	}

	int cont_temp_row=0;
	
	char* temp_row=(char*)malloc(sizeof(char) * size_read);
	if(temp_row == NULL){
		printf("Fail to alloc mem for temp_row in UNIQ_THREAD_UNIQ_NON_SENS \n");
		pthread_exit(0);
	}
	
	char buff;
	char* prev_row=(char *)calloc(sizeof(char),2);
	int cont_temp_row_resized=1;
	
	while(read(id,&buff,1))
	{
		if(cont_temp_row == (size_read) -1)
		{
			temp_row[cont_temp_row]=0;
			char* aux=temp_row;
			
			cont_temp_row_resized++;
			
			temp_row=(char*)malloc(sizeof(char) * size_read * cont_temp_row_resized);
			if(temp_row == NULL){
				printf("Fail to alloc mem for temp_row in UNIQ_THREAD_UNIQ_NON_SENS \n");
				pthread_exit(0);
			}
			
			strcpy(temp_row,aux);
			free(aux);
			
			cont_temp_row++;
		}
		
		if(buff == '\n' || buff == EOF)
		{
			temp_row[cont_temp_row]=0;
		
			/* if cur row if different from the above one */
			{
				if(prev_row != NULL)
				{
					if(stricmp(temp_row,prev_row) != 0)
						printf("%s \n",temp_row);
				}
				else
					printf("%s\n",temp_row);
				
				free(prev_row);
				prev_row=temp_row;
			
				cont_temp_row=0;
				cont_temp_row_resized=1;

				temp_row=(char*)malloc(sizeof(char) * size_read * cont_temp_row_resized);
				if(temp_row == NULL){
					printf("Fail to alloc mem for temp_row in UNIQ_THREAD_UNIQ_NON_SENS \n");
					pthread_exit(0);
				}
			}
			
		}
		else
			temp_row[cont_temp_row++]=buff;
		
	}
	
	free(prev_row);
	free(temp_row);
	
	close(id);
	
	pthread_exit(0);
}

void* uniq_thread_uniq_sensitive(void *file_arg) /*care trebe schimbat */
{
	char* file=(char *)file_arg;
	
	
	
	int test=test_if_file(file);
	if(test == 3){
		printf("uniq: %s: No such file or directory \n",file);
		pthread_exit(0);
	}
	if(test == 2){
		printf("uniq: %s: Is a directory directory",file);
		pthread_exit(0);
	}
	
	int id=open(file,O_RDONLY);
	if(id < 0){
		printf("Fail to open file in UNIQ_THREAD_DEFAULT \n");
		pthread_exit(0);
	}

	int cont_temp_row=0;
	
	char* temp_row=(char*)malloc(sizeof(char) * size_read);
	if(temp_row == NULL){
		printf("Fail to alloc mem for temp_row in UNIQ_THREAD_DEFAULT \n");
		pthread_exit(0);
	}
	
	char buff;
	char* prev_row=(char *)calloc(sizeof(char),2);
	int cont_temp_row_resized=1;
	int repeated=-1;
	
	while(read(id,&buff,1))
	{
		if(cont_temp_row == (size_read) -1)
		{
			temp_row[cont_temp_row]=0;
			char* aux=temp_row;
			
			cont_temp_row_resized++;
			
			temp_row=(char*)malloc(sizeof(char) * size_read * cont_temp_row_resized);
			if(temp_row == NULL){
				printf("Fail to alloc mem for temp_row in UNIQ_THREAD_UNIQ_NON_SENS \n");
				pthread_exit(0);
			}
			
			strcpy(temp_row,aux);
			free(aux);
			
			cont_temp_row++;
		}
		
		if(buff == '\n' || buff == EOF)
		{
			temp_row[cont_temp_row]=0;
		
			{
				if(prev_row == NULL)
				{
					prev_row=temp_row;
					repeated=0;
				}
				else
				{
					if(strcmp(temp_row,prev_row) == 0)
						repeated=1;
					else
					{
						if(repeated == 0)
							printf("%s \n",prev_row);
						
							
						repeated=0;
					}
					
					free(prev_row);
					prev_row=temp_row;
			
					cont_temp_row=0;
					cont_temp_row_resized=1;
						
				}
				
				temp_row=(char*)malloc(sizeof(char) * size_read * cont_temp_row_resized);
				if(temp_row == NULL){
					printf("Fail to alloc mem for temp_row in UNIQ_THREAD_UNIQ_NON_SENS \n");
					pthread_exit(0);
				}
			}
			
		}
		else
			temp_row[cont_temp_row++]=buff;
		
	}
	
	if(repeated == 0)
		printf("%s \n",prev_row);
		
	free(prev_row);
	free(temp_row);
	
	close(id);
	
	pthread_exit(0);
}

void* uniq_thread_uniq_non_sensitive(void *file_arg) /*care trebe schimbat */
{
	char* file=(char *)file_arg;
	
	
	
	int test=test_if_file(file);
	if(test == 3){
		printf("uniq: %s: No such file or directory \n",file);
		pthread_exit(0);
	}
	if(test == 2){
		printf("uniq: %s: Is a directory directory",file);
		pthread_exit(0);
	}
	
	int id=open(file,O_RDONLY);
	if(id < 0){
		printf("Fail to open file in UNIQ_THREAD_DEFAULT \n");
		pthread_exit(0);
	}

	int cont_temp_row=0;
	
	char* temp_row=(char*)malloc(sizeof(char) * size_read);
	if(temp_row == NULL){
		printf("Fail to alloc mem for temp_row in UNIQ_THREAD_DEFAULT \n");
		pthread_exit(0);
	}
	
	char buff;
	char* prev_row=(char *)calloc(sizeof(char),2);
	int cont_temp_row_resized=1;
	int repeated=-1;
	
	while(read(id,&buff,1))
	{
		if(cont_temp_row == (size_read) -1)
		{
			temp_row[cont_temp_row]=0;
			char* aux=temp_row;
			
			cont_temp_row_resized++;
			
			temp_row=(char*)malloc(sizeof(char) * size_read * cont_temp_row_resized);
			if(temp_row == NULL){
				printf("Fail to alloc mem for temp_row in UNIQ_THREAD_UNIQ_NON_SENS \n");
				pthread_exit(0);
			}
			
			strcpy(temp_row,aux);
			free(aux);
			
			cont_temp_row++;
		}
		
		if(buff == '\n' || buff == EOF)
		{
			temp_row[cont_temp_row]=0;
		
			{
				if(prev_row == NULL)
				{
					prev_row=temp_row;
					repeated=0;
				}
				else
				{
					if(stricmp(temp_row,prev_row) == 0)
						repeated=1;
					else
					{
						if(repeated == 0)
							printf("%s \n",prev_row);
						
							
						repeated=0;
					}
					
					free(prev_row);
					prev_row=temp_row;
			
					cont_temp_row=0;
					cont_temp_row_resized=1;
						
				}
				
				temp_row=(char*)malloc(sizeof(char) * size_read * cont_temp_row_resized);
				if(temp_row == NULL){
					printf("Fail to alloc mem for temp_row in UNIQ_THREAD_UNIQ_NON_SENS \n");
					pthread_exit(0);
				}
			}
			
		}
		else
			temp_row[cont_temp_row++]=buff;
		
	}
	
	if(repeated == 0)
		printf("%s \n",prev_row);
		
	free(prev_row);
	free(temp_row);
	
	close(id);
	
	pthread_exit(0);
}

void uniq(char **args, char* path_process_base)
{
	if(args[1] == NULL)
	{
		char *path_file_buffer="TempFileUniqx01.txt";
		int TempBuffFile = open(path_file_buffer, O_RDWR|O_CREAT|O_APPEND, 0600);
		
		if(TempBuffFile < 0)
		{
			printf("A error has occur durring the creating the buffer file in TAIL \n");
			exit(7);
		}
		
		char *temp_buff=(char*)malloc(sizeof(char) * (size_read) + 1);
		if(temp_buff == NULL){
			printf("Fail to alloc mem for buff in UNIQ \n");
			exit(2);
		}
		
		int read_size;
		while((read_size=read(0,temp_buff,size_read)))
			write(TempBuffFile,temp_buff,read_size);
			
		close(TempBuffFile);
		
		pthread_t thread_1;
		
		pthread_create(&thread_1,NULL,uniq_thread_default_sensitive,path_file_buffer);
		
		pthread_join(thread_1,NULL);
			
		remove(path_file_buffer);
	}
	else
	{
		char duplicate=0; /* -d */
		char case_sensitive=1; /* -i */
		char unique=0; /* -u */
		
		char **file_list=(char**)malloc(sizeof(char*)*size_command);
		if(file_list == NULL){
			printf("fail to alloc mem for file_list in UNIQ \n");
			exit(2);
		}
		
		int cont_file_list=0;
		
		int cont=1;
		while(args[cont] != NULL)
		{
			if(strcmp(args[cont],"-u") == 0)
			{
				unique=1;
				cont++;
				continue;
			}
			if(strcmp(args[cont],"-i") == 0)
			{
				case_sensitive=0;
				cont++;
				continue;
			}
			if(strcmp(args[cont],"-d") == 0)
			{
				duplicate=1;
				cont++;
				continue;
			}
			
			file_list[cont_file_list]=(char*)malloc(sizeof(char) * strlen(args[cont]) +1);
			if(file_list[cont_file_list] == NULL){
				printf("Fail to alloc mem in UNIQ while \n");
				exit(2);
			}
			
			strcpy(file_list[cont_file_list++],args[cont]);
			cont++;
		}

		if(cont_file_list > 1) /* uniq command doesn't run if i pass more than one argument */
			exit(0);
			
		if(cont_file_list == 0)
		{
			if(duplicate == 1 && unique == 1)
				exit(0);
				
			char *path_file_buffer="TempFileUniqx01.txt";
			int TempBuffFile = open(path_file_buffer, O_RDWR|O_CREAT|O_APPEND, 0600);
		
			if(TempBuffFile < 0)
			{
				printf("A error has occur durring the creating the buffer file in TAIL \n");
				exit(7);
			}
		
			char *temp_buff=(char*)malloc(sizeof(char) * (size_read) + 1);
			if(temp_buff == NULL){
				printf("Fail to alloc mem for buff in UNIQ \n");
				exit(2);
			}
		
			int read_size;
			while((read_size=read(0,temp_buff,size_read)))
				write(TempBuffFile,temp_buff,read_size);
			
			close(TempBuffFile);
				
			pthread_t thread_1;
			
			if(case_sensitive == 0)
			{
				if(duplicate)
					pthread_create(&thread_1,NULL,uniq_thread_duplicate_non_sensitive,path_file_buffer);
				else if(unique)
					pthread_create(&thread_1,NULL,uniq_thread_uniq_non_sensitive,path_file_buffer);
				else
					pthread_create(&thread_1,NULL,uniq_thread_default_non_sensitive,path_file_buffer);
			}
			else
			{
				if(duplicate)
					pthread_create(&thread_1,NULL,uniq_thread_duplicate_sensitive,path_file_buffer);
				else if(unique)
					pthread_create(&thread_1,NULL,uniq_thread_uniq_sensitive,path_file_buffer);
				else
					pthread_create(&thread_1,NULL,uniq_thread_default_sensitive,path_file_buffer);
				
			}

			pthread_join(thread_1,NULL);
			
			remove(path_file_buffer);
		
		}
		else
		{	
			char* test=(char*)(malloc(sizeof(char)*strlen(file_list[0]) +1));
			strcpy(test,file_list[0]);
		
		
			pthread_t thread_1;
		
			if(duplicate == 1 && unique == 1)
				exit(0);
		
			if(case_sensitive == 0)
			{
				if(duplicate)
					pthread_create(&thread_1,NULL,uniq_thread_duplicate_non_sensitive,test);
				else if(unique)
					pthread_create(&thread_1,NULL,uniq_thread_uniq_non_sensitive,test);
				else
					pthread_create(&thread_1,NULL,uniq_thread_default_non_sensitive,test);
			}
			else
			{
				if(duplicate)
					pthread_create(&thread_1,NULL,uniq_thread_duplicate_sensitive,test);
				else if(unique)
					pthread_create(&thread_1,NULL,uniq_thread_uniq_sensitive,test);
				else
					pthread_create(&thread_1,NULL,uniq_thread_default_sensitive,test);
				
			}

			pthread_join(thread_1,NULL);
		}
		
	}
}

int process_command(char** curr_path, char** args, char** history, int* pos_history, int input, int first, int last, char* path_process_base, char* input_file_redirect , char *output_file_redirect)
{
	if(args != NULL)
		{
		char temp_args0[50];
		strcpy(temp_args0,args[0]);
		
		if(strcmp(args[0],"exit") == 0)
			exit(0);
		if(strcmp(temp_args0,"cd") == 0)
		{
			change_directory(curr_path,args);
			return -1;
		}
		
		int pip[2];
				
		
		if(pipe(pip) < 0){
			printf("Fail to create pipe \n");
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
			/*pipes*/
			if (first==1 && last==0 && input==0) 
			{
				dup2( pip[1], 1 );
			}
			else if (first==0 && last==0 && input!=0) 
			{
				dup2(input, 0);
				dup2(pip[1], 1);
			} 
			else 
			{
				dup2(input, 0);
			}
			
			
			/* redirects */
			if(input_file_redirect != NULL)
                {                    
                   int in_id=open(input_file_redirect,O_RDONLY, 0);
					if (in_id < 0)
					{
					printf("Failed to open %s input in REDIRECT\n", input_file_redirect);
					exit(7);
					}
					dup2(in_id, 0);
					close(in_id);
                }
			if(output_file_redirect != NULL)
			  {
				int out_id= creat(output_file_redirect, 0644);
                   if (out_id < 0)
                   {
                     printf("Failed to open %s output in REDIRECT\n", output_file_redirect);
                     exit(7);
                   }
                   dup2(out_id, 1);
                   close(out_id);
			  }
			
			
			if(strcmp(args[0],"history") == 0)
			{
				display_history(args,history,pos_history);
				exit(0);
			}
			if(strcmp(args[0],"tail") == 0)
			{
				tail(args,path_process_base);
				exit(0);
			}
			if(strcmp(args[0],"uniq") == 0)
			{
				uniq(args,path_process_base);
				exit(0);
			}
			else
			{
				chdir(*curr_path);
				if(execvp(args[0],args) < 0)
					exit(-1);
				exit(0);
			}
		}
		else{ /* parent */
			
			waitpid(id,&status_pid,0);
			
			if(status_pid != 0)
			{
				int cont=1;
				printf("%s ",temp_args0);
				while(args[cont] != NULL)
					printf("%s ",args[cont++]);
					
				printf(":command not found \n");
			}
			
	
			if(last == 1)
			  close(pip[0]);
			if(input != 0) 
		      close(input);
			close(pip[1]);
			
			//clean_args(args);
			
			return pip[0];
		}
	}	
	///################################################################################
	return -1;
}
