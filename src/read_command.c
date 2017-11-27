#include "../headers/read_command.h"
#include "../headers/file_operation.h"


void clear_history_if_full (int* pos, char** hist)
{
	if(*pos == size_history)
	{
		int i;
		for(i=0; i<size_history-1000; i++)
			hist[i]=hist[i + 1000];

		*pos = size_history-1000;
	}
}

void inset_to_history(int *pos, char** history, char* to_insert)
{
	clear_history_if_full(pos,history);

	history[*pos]=(char *)malloc(sizeof(char) * (strlen(to_insert)+1));
	if(history[*pos] == NULL){
		printf("fail to alloc memory for history");
		exit(2);
	}

	strcpy(history[*pos],to_insert);
	(*pos)++;
}

char* clean_command_sign(char* command) /* clean command of \ signs */
{
	char* temp=(char*)malloc(sizeof(char) * size_command);
	if(temp == NULL){
		printf("fail to alloc mem in clean_command_sign \n");
		exit(2);
	}
	
	strcpy(temp,command);
	
	int i=0;
	while(temp[i] != 0)
	{
		if(temp[i] == 92)
			strcpy(temp+i,temp+i+1);
			
		i++;
	}
	
	return temp;
}

char** parse_command(char* buff_command)
{
	char** args;
	args=(char **)malloc(sizeof(char *)* size_command);
	if(args == NULL){
		printf("Fail to alloc mem for args_command \n");
		exit(2);
	}
	
	args[0]=(char *)malloc(sizeof(char) * size_command);
	if(args[0] == NULL){
		printf("Fail to alloc mem for args_command[0] \n");
		exit(2);
	}
	
	int cont_args=0;
	int pos_args=0;
	int cont=0;
	short int special_case=0; /* fot the case of parsing a string which contain " -text- " */
	
	while((buff_command[cont] == ' ' || buff_command[cont] == '\t') && buff_command[cont] != 0)   /* skip blank spaces */
		cont++;
	
	while(buff_command[cont] != 0)
	{
		if(buff_command[cont] == '"')
		{
			special_case=1;
			cont++;
			
			while(buff_command[cont] != 0 && special_case)
			{
				if(buff_command[cont] != '"')
				{
					args[pos_args][cont_args]=buff_command[cont];
					cont++;
					cont_args++;
					args[pos_args][cont_args]=0;
				}
				else
				{
					cont++;
					args[pos_args][cont_args]=0;
					pos_args++;
					cont_args=0;
					special_case=0;
					
					args[pos_args]=(char *)malloc(sizeof(char) * size_command);
					if(args[0] == NULL){
						printf("Fail to alloc mem for args_command[pos_args] \n");
						exit(2);
					}
				}
			}
			
			while((buff_command[cont] == ' ' || buff_command[cont] == '\t') && buff_command[cont] != 0)   /* skip blank spaces */
				cont++;
		}
		if(buff_command[cont] != ' ' && buff_command[cont] != '\t' && special_case != 1 && buff_command[cont] != 0){
			args[pos_args][cont_args]=buff_command[cont];
			cont_args++;
			args[pos_args][cont_args]=0;
			cont++;
		}
		else
		{
			if(buff_command[cont] != 0)
			{
				args[pos_args][cont_args]=0;
				pos_args++;
				cont_args=0;
			
				while((buff_command[cont] == ' ' || buff_command[cont] == '\t') && buff_command[cont] != 0)   /* skip blank spaces */
					cont++;
				
				if(buff_command[cont] == 0)
					args[pos_args]=NULL;
				
				args[pos_args]=(char *)malloc(sizeof(char) * size_command);
				if(args[0] == NULL){
					printf("Fail to alloc mem for args_command[pos_args] \n");
					exit(2);
				}
			}
		}
	}
	
	
	
	//~ if(cont > 0 && (buff_command[cont] != ' ' || buff_command[cont] != '\t'))
	//~ {
		//~ args[pos_args][cont_args]=0;
		//~ pos_args++;
	//~ }
		
	
	if(args[pos_args] != NULL && strlen(args[pos_args])>0)
		args[++pos_args]=NULL;
	else
		args[pos_args]=NULL;
		
		int i;
	for(i=0; i<pos_args; i++)
		printf("%s \n",args[i]);
		
	//~ int i=0;
	//~ while(args[i] != NULL)
		//~ printf("%s \n",args[i++]);
		
	
			
	return args;
}

char* get_line_command_case_up(int* pos_history, char ** history, char* curr_path)
{
	 system ("/bin/stty raw");		/* normally when used getc u need to press the key, and after that u need to press enter to read that key
										*Using this command the system'll read every key without needing to press enter
										*This is useful in case the user press up or down key*/

	char *buff_command=(char *)malloc(sizeof(char) * size_command);
	int pos_buff=0;

	int *pos_history_up_down=(int *)malloc(sizeof(int));
	if(pos_history_up_down == NULL){
		printf("Fail to alloc mem ");
		exit(2);
	}

	int pos_buff_left_right=-1;

	if((*pos_history)-2 > 0)
		*pos_history_up_down= (*pos_history)-2;

	if(buff_command == NULL){
		printf("fail to alloc for buffer command");
		system ("/bin/stty cooked");
		exit(2);
	}

	printf("%c[2K", 27);
	strcpy(buff_command,history[*pos_history_up_down]);
	pos_buff=strlen(buff_command);
	printf("\r%s $> %s",curr_path,buff_command);

start_2: while(1){

		char cur_char;

		cur_char=getc(stdin);

	if(cur_char == '\033'){
		getc(stdin);
		switch(getc(stdin)){

			case 'A':  /* for reading up key */
			{				//printf("ceva");
				printf("%c[2K", 27);
				if(*pos_history_up_down>0)
					(*pos_history_up_down)--;

				strcpy(buff_command,history[*pos_history_up_down]);
				pos_buff=strlen(buff_command);
				printf("\r%s $> %s",curr_path,buff_command);
				pos_buff_left_right=-1;
				goto start_2;
				break;
			}
			case 'B':  /* down key */
			{
				printf("%c[2K", 27);
				if(*pos_history_up_down< (*pos_history)-1)
					(*pos_history_up_down)++;

				strcpy(buff_command,history[*pos_history_up_down]);
				pos_buff=strlen(buff_command);
				printf("\r%s $> %s",curr_path,buff_command);
				pos_buff_left_right=-1;
				goto start_2;
				break;
			}
			case 'C': /* right key */
			{
				buff_command[pos_buff]=0;
				printf("%c[2K", 27);
				printf("\r%s $> %s",curr_path,buff_command);

				if(pos_buff_left_right != -1 && pos_buff_left_right < pos_buff)
				{
					pos_buff_left_right++;

					if(pos_buff_left_right == pos_buff)
					{
						cursorforward(0);
						cursorbackward(0);
					}
					else
						cursorbackward(difference);
				}
				goto start_2;
				break;
			}
			case 'D': /* left key */
			{
				buff_command[pos_buff]=0;
				printf("%c[2K", 27);
				printf("\r%s $> %s",curr_path,buff_command);

				if(pos_buff_left_right == -1)
					pos_buff_left_right=pos_buff-1;
				else
					if(pos_buff_left_right > 0)
						pos_buff_left_right--;
				cursorbackward(difference);
				goto start_2;
				break;
			}
		}
	}


	/* case user press enter */
	/* i used 13 instead of \n because the VM had reassigned some keys from my keyboard */
	if(cur_char == 13 || cur_char == EOF){
		printf("%c[2K", 27);  //erase current line because getc'll print the enter at the end of the string and i don't want that
		buff_command[pos_buff]=0;

		/* set system back to normal because we don't want to keep it in that state */
		system ("/bin/stty cooked");

		printf("\r%s $> %s",curr_path,buff_command);  /* \r returns the carriage the the begining of the current line */
		if(*pos_history > 0)
			(*pos_history)--;

		pos_buff_left_right=-1;
		return buff_command;
	}

	/* case backspace */
		if(cur_char == 127){

			if(pos_buff_left_right == -1 || pos_buff_left_right == pos_buff){
				if(pos_buff_left_right == pos_buff && pos_buff > 0)
					pos_buff_left_right--;

				if(pos_buff > 0)
					pos_buff--;

				buff_command[pos_buff]=0;
				printf("%c[2K", 27);
				printf("\r%s $> %s",curr_path,buff_command);
			}
			else{
				if(pos_buff_left_right > 0){
					buff_command[pos_buff]=0;
					strcpy(buff_command+pos_buff_left_right-1,buff_command+pos_buff_left_right);

					if(pos_buff > 0)
						pos_buff--;

					if(pos_buff_left_right > 0)
						pos_buff_left_right--;

					printf("%c[2K", 27);
					printf("\r%s $> %s",curr_path,buff_command);
					cursorbackward(difference);
				}
				else
				{
					printf("%c[2K", 27);
					printf("\r%s $> %s",curr_path,buff_command);
					cursorbackward(difference);
				}
			}
		}


	else
	{
		if(pos_buff == 299)  /* if buff command full */
		{
			char *temp=(char *)malloc(sizeof(char) * size_command);
			if(temp == NULL){
				printf("fail to alloc mem for buff");
				system ("/bin/stty cooked");
				exit(2);
			}

			buff_command[pos_buff]=0;
			strcpy(temp,buff_command);

			free(buff_command);
			buff_command=(char *)malloc(size_command*2*sizeof(char));

			if(buff_command == NULL){
				printf("fail to alloc for buffer command");
				system ("/bin/stty cooked");
				exit(2);
			}

			strcpy(buff_command,temp);
		}

		if(pos_buff_left_right != -1)
		{
			char *temp=(char *)malloc(sizeof(char) * size_command);
			if(temp == NULL){
				printf("fail to alloc mem for buff");
				system ("/bin/stty cooked");
				exit(2);
			}


			buff_command[pos_buff]=0;
			strcpy(temp, buff_command+pos_buff_left_right);
			buff_command[pos_buff_left_right]=cur_char;
			strcpy(buff_command+pos_buff_left_right+1, temp);
			pos_buff_left_right++;
			printf("%c[2K", 27);
			printf("\r%s $> %s",curr_path,buff_command);

			if(pos_buff_left_right-1 == pos_buff)
			{
				cursorforward(0);
				cursorbackward(0);
			}
			else
				cursorbackward(difference+1);
		}
		else
			buff_command[pos_buff]=cur_char;

		pos_buff++;
	}
	}///end while--------------------------------------------------------------------------------------------------------------------------------------------------------
}

char* get_line_command(int* pos_history, char ** history, char* curr_path)
{
	 system ("/bin/stty raw");		/* normally when used getc u need to press the key, and after that u need to press enter to read that key
										*Using this command the system'll read every key without needing to press enter
										*This is useful in case the user press up or down key*/

	char *buff_command=(char *)malloc(sizeof(char) * size_command);
	int pos_buff=0;
	int pos_buff_left_right=-1;

	if(buff_command == NULL){
		printf("fail to alloc for buffer command");
		system ("/bin/stty cooked");
		exit(2);
	}

 start:	while(1){

		char cur_char;

		cur_char=getc(stdin);

		if(cur_char == '\033'){
			getc(stdin);
			switch(getc(stdin)){

			case 'A':  /* for reading up key */
			{
				buff_command[pos_buff]=0;					/* if press up save current buff */
				inset_to_history(pos_history,history,buff_command);
				strcpy(buff_command,"up");
				buff_command[2]=0;

				/* set system back to normal because we don't want to keep it in that state */
				system ("/bin/stty cooked");

				pos_buff_left_right=-1;
				return buff_command;
				break;
			}
			case 'B':  /* down key */
			{
				/* nothing to do, just read it */
				buff_command[pos_buff]=0;
				printf("%c[2K", 27);
				printf("\r%s $> %s",curr_path,buff_command);
				pos_buff_left_right=-1;
				goto start;
				break;
			}
			case 'C': /* right key */
			{
				buff_command[pos_buff]=0;
				printf("%c[2K", 27);
				printf("\r%s $> %s",curr_path,buff_command);

				if(pos_buff_left_right != -1 && pos_buff_left_right < pos_buff)
				{
					pos_buff_left_right++;

					if(pos_buff_left_right == pos_buff)
					{
						cursorforward(0);
						cursorbackward(0);
					}
					else
						cursorbackward(difference);
				}
				goto start;
				break;
			}
			case 'D': /* left key */
			{
				buff_command[pos_buff]=0;
				printf("%c[2K", 27);
				printf("\r%s $> %s",curr_path,buff_command);

				if(pos_buff_left_right == -1)
					pos_buff_left_right=pos_buff-1;
				else
					if(pos_buff_left_right > 0)
						pos_buff_left_right--;
				cursorbackward(difference);
				goto start;
				break;
			}
			}
		}

	/* case user press enter */
	/* i used 13 instead of \n because the VM had reassigned some keys from my keyboard */
	if(cur_char == 13 || cur_char == EOF){
		printf("%c[2K", 27);  //erase current line because getc'll print the enter at the end of the string and i don't want that
		buff_command[pos_buff]=0;

		/* set system back to normal because we don't want to keep it in that state */
		system ("/bin/stty cooked");

		printf("\r%s $> %s",curr_path,buff_command);  /* \r returns the carriage the the begining of the current line */
		pos_buff_left_right=-1;
		return buff_command;
	}

	/* case backspace */
	if(cur_char == 127){

		if(pos_buff_left_right == -1 || pos_buff_left_right == pos_buff){

			if(pos_buff_left_right == pos_buff && pos_buff > 0)
				pos_buff_left_right--;

			if(pos_buff > 0)
				pos_buff--;

			buff_command[pos_buff]=0;
			printf("%c[2K", 27);
			printf("\r%s $> %s",curr_path,buff_command);
		}
		else{

			if(pos_buff_left_right > 0){
				buff_command[pos_buff]=0;
				strcpy(buff_command+pos_buff_left_right-1,buff_command+pos_buff_left_right);

				if(pos_buff > 0)
					pos_buff--;

				if(pos_buff_left_right > 0)
					pos_buff_left_right--;

				printf("%c[2K", 27);
				printf("\r%s $> %s",curr_path,buff_command);
				cursorbackward(difference);
			}
			else
			{
				printf("%c[2K", 27);
				printf("\r%s $> %s",curr_path,buff_command);
				cursorbackward(difference);
			}
		}
	}

	else
	{
		if(pos_buff == 299)  /* if buff command full */
		{
			char *temp=(char *)malloc(sizeof(char) * size_command);
			if(temp == NULL){
				printf("fail to alloc mem for buff");
				system ("/bin/stty cooked");
				exit(2);
			}

			buff_command[pos_buff]=0;
			strcpy(temp,buff_command);

			free(buff_command);
			buff_command=(char *)malloc(size_command*2*sizeof(char));

			if(buff_command == NULL){
				printf("fail to alloc for buffer command");
				system ("/bin/stty cooked");
				exit(2);
			}

			strcpy(buff_command,temp);
		}

		if(pos_buff_left_right != -1)
		{
			char *temp=(char *)malloc(sizeof(char) * size_command);
			if(temp == NULL){
				printf("fail to alloc mem for buff");
				system ("/bin/stty cooked");
				exit(2);
			}

			buff_command[pos_buff]=0;
			strcpy(temp, buff_command+pos_buff_left_right);
			buff_command[pos_buff_left_right]=cur_char;
			strcpy(buff_command+pos_buff_left_right+1, temp);
			pos_buff_left_right++;
			printf("%c[2K", 27);
			printf("\r%s $> %s",curr_path,buff_command);

			if(pos_buff_left_right-1 == pos_buff)
			{
				cursorforward(0);
				cursorbackward(0);
			}
			else
				cursorbackward(difference+1);
		}
		else
			buff_command[pos_buff]=cur_char;

		pos_buff++;
	}
	}///end while--------------------------------------------------------------------------------------------------------------------------------------------------------
}

int main_read_command()
{
	char **history;
	int pos_history=0;  /* keep track of history size */
	char* sign="$> ";
	char* command=NULL;
	char* curr_path=NULL;
	char* start_path="/home";
	char** args=NULL; /* command to be executed */
	char** table_command=(char**)malloc(sizeof(char*)*100);
	int input=0;
	
	if(table_command == NULL){
		printf("Fail to alloc mem for table_command main_read_command \n");
		exit(2);
	}
	
	init_path(start_path,&curr_path);
		
	/* try to alloc for history commands */
	history=(char **)malloc(sizeof(char *)*size_history);
	if(history == NULL){
		printf("fail to alloc for history table");
		return 1;
	}

	while(1){
		printf("%s %s",curr_path,sign);
		command=get_line_command(&pos_history,history,curr_path);

		if(strcmp(command,"up") == 0){
			command=get_line_command_case_up(&pos_history,history,curr_path);
		}

		command=clean_command_sign(command);
		if(strlen(command) > 0)
			inset_to_history(&pos_history,history,command);
			
		printf("\n");
		 
		int cont_commands=0;

		table_command[cont_commands++]=strtok(command,"|");
		while(table_command[cont_commands-1] != NULL)
			table_command[cont_commands++]=strtok(NULL,"|");
		
		if(cont_commands == 2)
		{
			args=parse_command(table_command[0]);
			input=process_command(&curr_path,args,history,&pos_history,input,0,1);
			//clean_args(args);
		}
		else
		{
			int i;
			for(i=0; i<cont_commands-1; i++)
			{
				args=parse_command(table_command[i]);
				if(i == 0)
				{
					input=process_command(&curr_path,args,history,&pos_history,input,1,0);
					//clean_args(args);
					continue;
				}
			    if(i == cont_commands-2)
				{
					input=process_command(&curr_path,args,history,&pos_history,input,0,1);
				}
				else
					input=process_command(&curr_path,args,history,&pos_history,input,0,0);
					
				//clean_args(args);
			}
			//clean_args(args);	
		}
		
			
		//args=parse_command(command);
		//process_command(&curr_path,args,history,&pos_history);
		input=0;
			
		if(strcmp(command,"exit") == 0)
			return 0;
	}
}
