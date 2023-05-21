/********************************************************************
** ctype.h
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
** This file is a mock for standard C header file ctype.h
*/

#include <ctype.h>

int iscntrl(int ch)
{
	return (ch < 32) || (ch == 127);
}

int isprint(int ch)
{
	return (32 <= ch) && (ch <= 126);
}

int isspace(int ch)
{
	return ((9 <= ch) && (ch <= 13)) || (ch == 32);
}

int isblank(int ch)
{
	return (ch == 9) || (ch == 32);
}

int isgraph(int ch)
{
	return (33 <= ch) && (ch <= 126);
}

int ispunct(int ch)
{
	return ((33 <= ch) && (ch <= 47)) ||
		((58 <= ch) && (ch <= 64)) ||
		((91 <= ch) && (ch <= 96));
}

int isalnum(int ch)
{
	return isalpha(ch) || isdigit(ch);
}

int isalpha(int ch)
{
	return isupper(ch) || islower(ch);
}

int isupper(int ch)
{
	return ('A' <= ch) && (ch <= 'Z');
}

int islower(int ch)
{
	return ('a' <= ch) && (ch <= 'z');
}

int isdigit(int ch)
{
	return ('0' <= ch) && (ch <= '9');
}
