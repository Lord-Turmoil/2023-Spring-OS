/********************************************************************
** lib.c
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
**   This file defines some auxiliary functions.
*/

#include <lib.h>
#include <ctype.h>


/*
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
** Directory related
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
int access(char* path)
{
	int fd = open(path, O_RDONLY);
	
	if (fd < 0)
		return 0;
	close(fd);

	return 1;
}

int getcwd(char* path)
{
	debugf("pwd 3: %s\n", path);
	return syscall_get_pwd(path);
}

/********************************************************************
** To avoid volatile attribute, make all operations to kernel.
*/
int chdir(const char* path)
{
	char dir[MAXPATHLEN];

	if (strlen(path) >= MAXPATHLEN)
		return -E_NOT_FOUND;
	while (*path && *path == ' ')
		path++;
	if (path[0] != '/')	// relative change
	{
		try(syscall_get_pwd(dir));
		
		if (strlen(dir) + strlen(path) + 1 >= MAXPATHLEN)
			return -E_NOT_FOUND;

		strcat(dir, "/");
		strcat(dir, path);
	}
	else
		strcpy(dir, path);

	if (!access(dir))
		return -E_NOT_FOUND;

	strstripr(dir, '/');	// remove trailing '/'
	syscall_set_pwd(dir);
	
	return 0;
}

/*
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
** Auxiliary functions
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
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
