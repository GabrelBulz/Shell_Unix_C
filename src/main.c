
/*
 * Error list
 * 2-Fail to alloc mem
 * 3-Fail to execut command
 * 4-Fail to fork
 * 5-Fail to create pipe
 * 6-Invalid arguments
 * 7-Fail to create file
 * */

#include <wait.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <string.h>
#include "../headers/file_operation.h"
#include "../headers/read_command.h"


#define size_history 5000

int main()
{
	
	if(main_read_command() == 0)
	{
		return 0;
	}
	else
		return 1;

}
