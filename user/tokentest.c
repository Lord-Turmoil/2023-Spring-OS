/********************************************************************
** tokentest.c
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
** Test file for token.
*/

#include <lib.h>
#include <arguments.h>
#include <pash.h>
#include <conio.h>

int main(int argc, char* argv[])
{
	printf("Token test begin!\n");

	input_opt_t opt;
	init_input_opt(&opt);
	opt.maxLen = 127;
	opt.interruptible = 1;

	char buffer[128];
	char* token;
	token_t type;

	while (get_string(buffer, &opt) != EOF)
	{
		printf("\n");
		type = get_token(buffer, &token);
		if (token)
			printf("\t[%d] - '%s'\n", type, token);
		else
			printf("\t[%d] - 'NULL'\n", type);
		
		for (; ; )
		{
			type = get_token(NULL, &token);
			if (token)
				printf("\t[%d] - '%s'\n", type, token);
			else
				printf("\t[%d] - 'NULL'\n", type);
			if (type == TK_EMPTY)
				break;
		}

		printf("--------------------\n");
		if (is_the_same(buffer, "q"))
			break;
	}

	return 0;
}
