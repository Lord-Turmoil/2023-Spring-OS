/********************************************************************
** lib.c
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
**   This file defines some auxiliary functions.
*/

#include <lib.h>
#include <ctype.h>

int access(const char* path, int type)
{
	char dir[MAXPATHLEN];

	struct Stat st;
	int ret;

	while (*path && (*path == ' '))
		path++;
	if (path[0] == '/')	// absolute
		strcpy(dir, path);
	else
	{
		getcwd(dir);
		if (strcmp(dir, "/") != 0)
			strcat(dir, "/");
		strcat(dir, path);
	}

	ret = stat(dir, &st);
	if (ret < 0)
		return 0;

	if (type == FTYPE_DIR)
		return st.st_isdir;
	else if (type == FTYPE_REG)
		return !st.st_isdir;

	return 1;
}

void getcwd(char* path)
{
	syscall_get_pwd(path);
}

int chdir(const char* path)
{
	char dir[MAXPATHLEN];

	if (strlen(path) >= MAXPATHLEN)
		return -E_NOT_FOUND;

	while (*path && *path == ' ')
		path++;

	if (path[0] != '/')     // relative change
	{
		syscall_get_pwd(dir);

		if (strlen(dir) + strlen(path) + 1 >= MAXPATHLEN)
			return -E_NOT_FOUND;

		strcat(dir, "/");
		strcat(dir, path);
	}
	else
		strcpy(dir, path);

	if (!access(dir, FTYPE_DIR))
		return -E_NOT_FOUND;

	if (strcmp(dir, "/") != 0)
		strstripr(dir, '/');    // remove trailing '/'

	syscall_set_pwd(dir);

	return 0;
}

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

int isdir(const char* path)
{
	struct Stat st;

	if (stat(path, &st) < 0)
		return 0;

	return st.st_isdir;
}

int isreg(const char* path)
{
	struct Stat st;

	if (stat(path, &st) < 0)
		return 0;

	return !st.st_isdir;
}

int is_the_same(const char* str1, const char* str2)
{
	// if both are NULL, return 0

	if (!!str1 && !!str2)
		return strcmp(str1, str2) == 0;
	return 0;
}

int is_null_or_empty(const char* str)
{
	return !str || !*str;
}

int is_no_content(const char* str)
{
	if (is_null_or_empty(str))
		return 1;
	for (const char* p = str; *p; p++)
	{
		if (isprint(*p))
			return 0;
	}

	return 1;
}

int is_begins_with(const char* str, const char* prefix)
{
	if (is_null_or_empty(prefix))
		return 1;
	if (is_null_or_empty(str))
		return 0;

	while (*prefix && *str)
	{
		if (*prefix != *str)
			return 0;
		prefix++;
		str++;
	}

	return !*prefix;
}

int is_ends_with(const char* str, const char* suffix)
{
	if (is_null_or_empty(suffix))
		return 1;
	if (is_null_or_empty(str))
		return 0;

	const char* str_end = str + strlen(str);
	const char* suffix_end = suffix + strlen(suffix);

	while (str_end >= str && suffix_end >= suffix)
	{
		if (*str_end != *suffix_end)
			return 0;
		str_end--;
		suffix_end--;
	}

	return suffix_end < suffix;
}
