/********************************************************************
** amp.c
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
**   This file is used to demonstrate '&' in command.
*/

#include <lib.h>

// ampersand 1000 a
int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf("Usage: amp n c\n");
		return 1;
	}

	int delay = atoi(argv[1]);
	if (delay == 0)
		delay = 1000;

	sleep(delay);

	printf("%s\n", argv[2]);

	return 0;
}
