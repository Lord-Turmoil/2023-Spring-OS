/********************************************************************
** ctype.h
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
** This file is a mock for standard C header file ctype.h
*/

#ifndef _CTYPE_H_
#define _CTYPE_H_

int iscntrl(int ch);
int isprint(int ch);
int isspace(int ch);
int isblank(int ch);
int isgraph(int ch);
int ispunct(int ch);
int isalnum(int ch);
int isalpha(int ch);
int isupper(int ch);
int islower(int ch);
int isdigit(int ch);

#endif