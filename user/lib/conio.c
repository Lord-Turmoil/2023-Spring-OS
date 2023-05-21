/********************************************************************
** conio.c
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
** This file declares the direct console I/O functions.
*/

#include <conio.h>
#include <lib.h>

int getch()
{
	return syscall_getch();
}

void putch(int ch)
{
	syscall_putchar(ch);
}