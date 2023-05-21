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
	struct Fd* fd;
	int ret;
	if ((ret = fd_lookup(0, &fd)) < 0)
		return EOF;

	int ch;
	if (fd->fd_dev_id == devcons.dev_id)
	{
		ch = syscall_getch();
		if (ch == -EOF)
			return EOF;
	}
	else
	{
		if (read(0, &ch, 1) != 1)
			return EOF;
	}

	return ch;
}

void putch(int ch)
{
	syscall_putchar(ch);
}