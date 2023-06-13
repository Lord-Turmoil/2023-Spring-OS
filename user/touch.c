/********************************************************************
** touch.c
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
**   Implements touch command.
*/

#include <lib.h>
#include <arguments.h>

static int enableParents;
static int enableVerbose;

static void init();
static void usage();
static int parse_args(int argc, char* argv[]);
static int touch(const char* path);

int main(int argc, char* argv[])
{
	init();

	if (parse_args(argc, argv) != 0)
	{
		usage();
		return 1;
	}

	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
			continue;

		touch(argv[i]);
	}

	return 0;
}

static void init()
{
	enableParents = 0;
	enableVerbose = 0;
}

static void usage()
{
	printfc(MSG_COLOR, "Usage: touch path [path ...]\n");
}

static int parse_args(int argc, char* argv[])
{
	int opt;
	int arg_cnt = 0;
	int err = 0;
	while ((opt = getopt(argc, argv, "v")))
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
		case 'v':
			enableVerbose = 1;
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

static int touch(const char* path)
{
	if (access(path, FTYPE_REG))
	{
		printfc(ERROR_COLOR, "Cannot create file '%s': File exists\n", path);
		return 11;
	}
	if (access(path, FTYPE_DIR))
	{
		printfc(ERROR_COLOR, "Cannot create file '%s': Directory exists\n", path);
		return 12;
	}

	// now, there is no such a file
	int ret = creat(path, 0);
	if (ret < 0)
	{
		printfc(ERROR_COLOR, "Failed to create '%s': %d\n", path, ret);
		return ret;
	}

	if (enableVerbose)
		printfc(MSG_COLOR, "Created file '%s'\n", path);

	return 0;
}
