/********************************************************************
** ls.c
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
**   This file implements ls command.
*/

#include <lib.h>
#include <arguments.h>

static int enableClassify;
static int enableLonglist;
static int selfOnly;
static int hasTarget;

static void init();
static void usage();
static int parse_args(int argc, char* argv[]);

static void ls(const char* path, const char* prefix);
static void _lsdir(const char* path, const char* prefix);
static void _ls(const char* prefix, u_int isdir, u_int size, const char* name);

int main(int argc, char* argv[])
{
	init();
	if (parse_args(argc, argv) != 0)
	{
		usage();
		return 1;
	}

	if (!hasTarget)
	{
		char dir[MAXPATHLEN];
		getcwd(dir);
		ls(dir, "");
	}
	else
	{
		for (int i = 1; i < argc; i++)
		{
			if (argv[i][0] == '-')
				continue;
			if (strcmp(argv[i], "/") == 0)
				ls(argv[i], argv[i]);
			else
				ls(argv[i], strstripr(argv[i], '/'));
		}
	}

	if (!enableLonglist)
		printf("\n");

	return 0;
}

static void init()
{
	enableClassify = 0;
	enableLonglist = 0;
	selfOnly = 0;
	hasTarget = 0;
}

static void usage()
{
	printfc(MSG_COLOR, "Usage: ls [-d -F -l] [path]\n");
}

static int parse_args(int argc, char* argv[])
{
	int opt;
	int err = 0;
	while ((opt = getopt(argc, argv, "dFl")))
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
		case 'd':
			selfOnly = 1;
			break;
		case 'F':
			enableClassify = 1;
			break;
		case 'l':
			enableLonglist = 1;
			break;
		case '!':
			hasTarget = 1;
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

static void ls(const char* path, const char* prefix)
{
	int ret;
	struct Stat st;

	if ((ret = stat(path, &st)) < 0)
	{
		printfc(ERROR_COLOR, "Cannot access '%s': No such file or directory\n", path);
		return;
	}

	if (st.st_isdir && !selfOnly)
		_lsdir(path, prefix);
	else
		_ls(NULL, st.st_isdir, st.st_size, path);
}

static void _lsdir(const char* path, const char* prefix)
{
	int fd;
	int size;
	struct File file;

	if ((fd = open(path, O_RDONLY)) < 0)
	{
		printfc(ERROR_COLOR, "Failed to open '%s': %d\n", path, fd);
		return;
	}

	while ((size = readn(fd, &file, sizeof(struct File))) == sizeof(struct File))
	{
		if (file.f_name[0])
			_ls(prefix, file.f_type == FTYPE_DIR, file.f_size, file.f_name);
	}
	if (size > 0)
		printfc(ERROR_COLOR, "Short read in directory '%s'\n", path);
	if (size < 0)
		printfc(ERROR_COLOR, "Error reading directory '%s': %d\n", path, size);
}

static void _ls(const char* prefix, u_int isdir, u_int size, const char* name)
{
	char* sep;
	int color;

	if (enableLonglist)
		printf("%11dB %11dKB  %c  ", size, size / 1024, isdir ? 'd' : '-');

	if (isdir)
		color = FOREGROUND_INTENSE(BLUE);
	else
	{
		if (is_ends_with(name, ".b"))
			color = FOREGROUND_INTENSE(GREEN);
		else
			color = FOREGROUND(WHITE);
	}

	if (prefix)
	{
		if (prefix[0] && prefix[strlen(prefix) - 1] != '/')
			sep = "/";
		else
			sep = "";
		printfc(color, "%s%s", prefix, sep);
	}

	printfc(color, "%s", name);
	if (isdir && enableClassify)
		printfc(color, "/");
	printf(" ");
	if (enableLonglist)
		printf("\n");
}
