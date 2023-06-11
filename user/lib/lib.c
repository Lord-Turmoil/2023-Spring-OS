/********************************************************************
** lib.c
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
**   This file defines some auxiliary functions.
*/

#include <ctype.h>

int atoi(const char* str)
{
	int ret = 0;
	int sign = 1;

	const char* p = str;
	while (p && !isdigit(*p))
	{
		if (*p == '-')
			sign = -1;
		p++;
	}
	while (p && isdigit(*p))
	{
		ret = ret * 10 + (*p) - '0';
		p++;
	}

	return ret * sign;
}
