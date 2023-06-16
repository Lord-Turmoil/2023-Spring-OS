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

	// debugf("Access: %s\n", dir);
	
	ret = stat(dir, &st);
	if (ret < 0)
	{
		// debugf("Acess error: %d\n", ret);
		return 0;
	}

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
			return -E_BAD_PATH;

		strcat(dir, "/");
		strcat(dir, path);
	}
	else
		strcpy(dir, path);

	if (!access(dir, FTYPE_DIR))
		return -E_NOT_DIR;

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


/********************************************************************
** Execute external command.
*/

int execl(const char* path, ...)
{
	char* argv[64];
	int argc = 0;
	char* arg;
	va_list args;

	// should assign path in argument again

	va_start(args, path);
	arg = va_arg(args, char*);
	while (arg)
	{
		argv[argc++] = arg;
		arg = va_arg(args, char*);
	}
	argv[argc] = NULL;

	return execv(path, argv);
}

int execv(const char* path, char* argv[])
{
	char prog[1024] = "/bin/";

	if (strchr(path, '/'))	// use directory to call command
		strcpy(prog, path);
	else
		strcat(prog, path);

	if (is_ends_with(prog, ".sh"))	// shell
	{
		// argv is abandoned
		return execl("pash", "pash", prog, NULL);
	}

	if (!is_ends_with(prog, ".b"))
		strcat(prog, ".b");

	return spawn(prog, argv);
}


// get user profile
static const char PROFILE_PATH[] = "/etc/passwd";

// will create old data
static int _creat_profile(const char* username, const char* path)
{
	if (!username || !path)
		return -1;

	int fd = open(PROFILE_PATH, O_WRONLY | O_CREAT);
	if (fd < 0)
		return -E_NOT_FOUND;

	write(fd, username, strlen(username));
	write(fd, ":", 1);
	write(fd, path, strlen(path));
	write(fd, "\n", 1);

	close(fd);

	return 0;
}

int profile(char* username, char* path, int create)
{
	int fd = open(PROFILE_PATH, O_RDONLY);
	if (fd < 0)
	{
		if (create)
			return _creat_profile(username, path);
		return fd;
	}

	char buffer[MAXPATHLEN];
	readline(fd, buffer);
	close(fd);

	strstripr(buffer, '\n');	// MUST!!!

	char* p;
	for (p = buffer; *p; p++)
	{
		if (*p == ':')
		{
			*(p++) = '\0';
			break;
		}
	}
	if (!*p)
		return -2;	// bad profile

	if (username)
		strcpy(username, buffer);
	if (path)
		strcpy(path, p);

	return 0;
}
