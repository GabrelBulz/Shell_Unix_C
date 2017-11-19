#include "read_command.h"
#include "file_operation.h"


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

char* get_line_command_case_up(int* pos_history, char ** history)
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
	printf("\r$> %s",buff_command);

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
				printf("\r$> %s",buff_command);
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
				printf("\r$> %s",buff_command);
				pos_buff_left_right=-1;
				goto start_2;
				break;
			}
			case 'C': /* right key */
			{
				buff_command[pos_buff]=0;
				printf("%c[2K", 27);
				printf("\r$> %s",buff_command);

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
				printf("\r$> %s",buff_command);

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

		printf("\r$> %s",buff_command);  /* \r returns the carriage the the begining of the current line */
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
				printf("\r$> %s",buff_command);
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
					printf("\r$> %s",buff_command);
					cursorbackward(difference);
				}
				else
				{
					printf("%c[2K", 27);
					printf("\r$> %s",buff_command);
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
			printf("\r$> %s",buff_command);

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

char* get_line_command(int* pos_history, char ** history)
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
				printf("\r$> %s",buff_command);
				pos_buff_left_right=-1;
				goto start;
				break;
			}
			case 'C': /* right key */
			{
				buff_command[pos_buff]=0;
				printf("%c[2K", 27);
				printf("\r$> %s",buff_command);

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
				printf("\r$> %s",buff_command);

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

		printf("\r$> %s",buff_command);  /* \r returns the carriage the the begining of the current line */
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
			printf("\r$> %s",buff_command);
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
				printf("\r$> %s",buff_command);
				cursorbackward(difference);
			}
			else
			{
				printf("%c[2K", 27);
				printf("\r$> %s",buff_command);
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
			printf("\r$> %s",buff_command);

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

	if(test_path("/home/gabrel/")) ///###########################################################################################################################
		printf("exista \n");
		
	/* try to alloc for history commands */
	history=(char **)malloc(sizeof(char *)*size_history);
	if(history == NULL){
		printf("fail to alloc for history table");
		return 1;
	}

	while(1){
		printf("%s",sign);
		command=get_line_command(&pos_history,history);

		if(strcmp(command,"up") == 0){
			command=get_line_command_case_up(&pos_history,history);
		}

		if(strcmp(command,"exit") == 0)
			return 0;

		if(strlen(command) > 0)
			inset_to_history(&pos_history,history,command);

		printf("\n");
	}
}
