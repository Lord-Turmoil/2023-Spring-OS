/********************************************************************
** tree.c
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
**   This file implements tree command. The original implementation
** comes from PassBash.
**   - https://github.com/Lord-Turmoil/PassBashPro
**   - https://gitee.com/tonys-studio/pass-bash-pro
*/

#include <lib.h>
#include <arguments.h>


static int dirOnly;
static char targetPath[MAXPATHLEN];

static void init();
static void usage();
static int parse_args(int argc, char* argv[]);
static void tree(const char* path);
static void _tree(
	const char* path,
	const char* name,
	const char* leading,
	int isend,
	int isdir);
static void _tree_dir(const char* path, const char* name, const char* leading);
static void _tree_reg(const char* path, const char* name, const char* leading);

int main(int argc, char* argv[])
{
	init();

	if (parse_args(argc, argv) != 0)
	{
		usage();
		return 1;
	}

	if (!is_the_same(targetPath, "/"))
		strstripr(targetPath, '/');
	tree(targetPath);

	return 0;
}

static void init()
{
	dirOnly = 0;
	strcpy(targetPath, ".");
}

static void usage()
{
	printfc(MSG_COLOR, "Usage: tree [-d] [path]\n");
}

static int parse_args(int argc, char* argv[])
{
	int opt;
	int arg_cnt = 0;
	int err = 0;
	while (opt = getopt(argc, argv, "d"))
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
			dirOnly = 1;
			break;
		case '!':
			arg_cnt++;
			if (arg_cnt == 1)
				strcpy(targetPath, optarg);
			else if (arg_cnt == 2)
			{
				err = 1;
				printfc(ERROR_COLOR, ERRMSG_TOO_MANY "\n");
			}
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

/********************************************************************
** This is a little tricky.
** To make it clear, '.' is for leading space.
** . current [leading "."]
** ...|-- alpha [leading ".'..|.."]
** ...|..  \-- beta [leading ".'..|..'.....]
** ...
*/
static void tree(const char* path)
{
	int ret;
	struct Stat st;

	if ((ret = stat(path, &st)) < 0)
	{
		printfc(ERROR_COLOR, "Cannot access '%s': No such directory\n", path);
		return;
	}

	if (!st.st_isdir)
	{
		printfc(ERROR_COLOR, "'%s' is not a directory\n", path);
		return;
	}

	_tree_dir(path, path, " ");
}

static void _tree(
	const char* path,
	const char* name,
	const char* leading,
	int isend,
	int isdir)
{
	char lead[MAXFILEDEPTH << 3];

	// construct leading
	printf("%s", leading);

	strcpy(lead, leading);
	if (isend)
	{
		strcat(lead, "     ");
		printf("  \\--");
	}
	else
	{
		strcat(lead, "  |  ");
		printf("  |--");
	}

	// next call
	if (isdir)
		_tree_dir(path, name, lead);
	else
		_tree_reg(path, name, lead);
}

static void _tree_dir(const char* path, const char* name, const char* leading)
{
	char dir[MAXPATHLEN];

	// display directory
	if (is_the_same(name, "/"))
		printfc(FOREGROUND_INTENSE(BLUE), " %s\n", name);
	else
		printfc(FOREGROUND_INTENSE(BLUE), " %s/\n", name);

	// open directory
	int fd;
	if ((fd = open(path, O_RDONLY)) < 0)
	{
		printfc(ERROR_COLOR, "Failed to open '%s': %d\n", path, fd);
		return;
	}

	// get number of sub items
	int size;
	struct File file;
	int cnt = 0;
	while ((size = readn(fd, &file, sizeof(struct File))) == sizeof(struct File))
	{
		if (!file.f_name[0])
			continue;

		strcpy(dir, path);
		strcat(dir, "/");
		strcat(dir, file.f_name);

		if (!dirOnly || isdir(dir))
			cnt++;
	}
	if (size > 0)
		printfc(ERROR_COLOR, "Short read in directory '%s'\n", path);
	if (size < 0)
		printfc(ERROR_COLOR, "Error reading directory '%s': %d\n", path, size);

	// tree items
	int i = 0;
	int is_dir;

	seek(fd, 0);
	while ((size = readn(fd, &file, sizeof(struct File))) == sizeof(struct File))
	{
		if (!file.f_name[0])
			continue;

		strcpy(dir, path);
		strcat(dir, "/");
		strcat(dir, file.f_name);

		is_dir = isdir(dir);
		if (dirOnly && !is_dir)
			continue;
		i++;
		_tree(dir, file.f_name, leading, (i == cnt), is_dir);
	}
	if (size > 0)
		printfc(ERROR_COLOR, "Short read in directory '%s'\n", path);
	if (size < 0)
		printfc(ERROR_COLOR, "Error reading directory '%s': %d\n", path, size);
}

static void _tree_reg(const char* path, const char* name, const char* leading)
{
	if (is_ends_with(name, ".b"))
		printfc(FOREGROUND_INTENSE(GREEN), " %s\n", name);
	else
		printfc(FOREGROUND(WHITE), " %s\n", name);
}
