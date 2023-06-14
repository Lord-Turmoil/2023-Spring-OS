/********************************************************************
** mkdir.c
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
**   Implements mkdir command.
*/

#include <lib.h>
#include <arguments.h>

static int enableParents;
static int enableVerbose;
static int showHelp;

static void init();
static void usage();
static int parse_args(int argc, char* argv[]);
static int mkdir(const char* path);
static int _create(const char* path, int create, int final);

int main(int argc, char* argv[])
{
	init();

	if (parse_args(argc, argv) != 0)
	{
		usage();
		return 1;
	}
	if (showHelp)
	{
		usage();
		return 0;
	}

	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
			continue;

		mkdir(argv[i]);
	}

	return 0;
}

static void init()
{
	enableParents = 0;
	enableVerbose = 0;
	showHelp = 0;
}

static void usage()
{
	printfc(MSG_COLOR, "Usage: mkdir [-pvh] path [path ...]\n");
}

static int parse_args(int argc, char* argv[])
{
	int opt;
	int err = 0;
	while ((opt = getopt(argc, argv, "pvh")))
	{
		if (opterr != 0)
		{
			printfc(ERROR_COLOR, "Argument error: %s\n", optmsg);
			err = 1;
			resetopt();
			break;
		}
		switch (opt)
		{
		case 'p':
			enableParents = 1;
			break;
		case 'v':
			enableVerbose = 1;
			break;
		case 'h':
			showHelp = 1;
			break;
		case '!':
			break;
		case '?':
			err = 1;
			printfc(ERROR_COLOR, "Unknown parameter \"-%c\"\n", optopt);
			break;
		default:
			break;
		}
	}

	if (err)
	{
		printfc(ERROR_COLOR, ERRMSG_ILLEGAL "\n");
		return 1;
	}

	return 0;
}

static int mkdir(const char* path)
{
	char dir[MAXPATHLEN];
	const char* p = path;
	const char* base = path;

	while (*p != '\0')
	{
		while (*p && (*p != '/'))
			p++;
		memcpy(dir, base, p - base);
		dir[p - base] = '\0';
		while (*p && (*p == '/'))
			p++;

		if (*p)	// there are remaining directories to create
			try(_create(dir, enableParents, 0));
		else	// the last element
			try(_create(dir, 1, 1));
	}

	return 0;
}

static int _create(const char* path, int create, int final)
{
	if (access(path, FTYPE_DIR))
	{
		if (!final)
			return 0;
		printfc(ERROR_COLOR, "Cannot create directory '%s': Directory exists\n", path);
		return 11;
	}
	if (access(path, FTYPE_REG))
	{
		printfc(ERROR_COLOR, "Cannot create directory '%s': File exists\n", path);
		return 12;
	}

	// now, there is no such a file
	if (!create && !final)
	{
		printfc(ERROR_COLOR, "Cannot create directory '%s': No such file or directory\n", path);
		return 13;
	}
	
	// now, it's time to create file
	int ret = creat(path, O_MKDIR);
	if (ret < 0)
	{
		printfc(ERROR_COLOR, "Failed to create '%s': %d\n", path, ret);
		return ret;
	}

	if (enableVerbose)
		printfc(MSG_COLOR, "Created directory '%s'\n", path);

	return 0;
}
