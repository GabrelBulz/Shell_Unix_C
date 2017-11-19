#include <wait.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <string.h>
#include "read_command.h"

#define size_history 5000

int main()
{
	
	if(main_read_command() == 0)
	{
		printf("\n");
		return 0;
	}
	else
		return 1;

}
