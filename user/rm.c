/********************************************************************
** rm.c
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
**   This file implements rm command.
*/

#include <lib.h>
#include <arguments.h>


static int enableForce;
static int enableVerbose;
static int enableRecursive;
static int showHelp;

static void init();
static void usage();
static int parse_args(int argc, char* argv[]);
static void rm(const char* path);
static void _rm(const char* path);

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

		if (strcmp(argv[i], "/") == 0)
		{
			printfc(ERROR_COLOR, "DON'T DO THAT!\n");
			printfc(MSG_COLOR, "     >_<`\n");
			return 1;
		}

		rm(strstripr(argv[i], '/'));
	}

	return 0;
}

static void init()
{
	enableForce = 0;
	enableRecursive = 0;
	enableVerbose = 0;
	showHelp = 0;
}

static void usage()
{
	printfc(MSG_COLOR, "Usage: rm [-vfrh] path [path ...]\n");
}

static int parse_args(int argc, char* argv[])
{
	int opt;
	int err = 0;
	while ((opt = getopt(argc, argv, "fvrh")))
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
		case 'f':
			enableForce = 1;
			break;
		case 'v':
			enableVerbose = 1;
			break;
		case 'r':
			enableRecursive = 1;
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

/********************************************************************
** This is similar to tree.
*/
static char* RESTRICTED_PATHS[] = {
	"/../",
	"/./",
	"../",
	"./",
	"/..",
	"/."
};

static void rm(const char* path)
{
	for (int i = 0; i < 6; i++)
	{
		if (strstr(path, RESTRICTED_PATHS[i]))
		{
			printfc(ERROR_COLOR, "Refusing to remove '.' or '..' directory: skipping '%s'\n", path);
			return;
		}
	}
	if (is_the_same(path, "..") || is_the_same(path, "."))
	{
		printfc(ERROR_COLOR, "Refusing to remove '.' or '..' directory: skipping '%s'\n", path);
		return;
	}


	int ret;
	struct Stat st;
	if ((ret = stat(path, &st)) < 0)
	{
		if (!enableForce)
			printfc(ERROR_COLOR, "Cannot access '%s': No such directory\n", path);
		return;
	}

	if (st.st_isdir && !enableRecursive)
	{
		printfc(ERROR_COLOR, "Cannot remove '%s': Is a directory\n", path);
		return;
	}

	if (st.st_isdir)
		_rm(path);
	
	ret = remove(path);
	if (ret != 0)
		printfc(ERROR_COLOR, "Failed to remove '%s': %d\n", path, ret);
	if (enableVerbose)
		printfc(MSG_COLOR, "Removed%s'%s'\n", st.st_isdir ? " directory " : " ", path);
}

// recursively remove a directory
static void _rm(const char* path)
{
	char dir[MAXPATHLEN];

	// open directory
	int fd;
	if ((fd = open(path, O_RDONLY)) < 0)
	{
		if (!enableForce)
			printfc(ERROR_COLOR, "Failed to open '%s': %d\n", path, fd);
		return;
	}

	// get number of sub items
	int size;
	int ret;
	int is_dir;
	struct File file;
	while ((size = readn(fd, &file, sizeof(struct File))) == sizeof(struct File))
	{
		if (!file.f_name[0])
			continue;

		strcpy(dir, path);
		strcat(dir, "/");
		strcat(dir, file.f_name);

		is_dir = isdir(dir);
		if (is_dir)
		{
			if (!enableRecursive)
				printfc(ERROR_COLOR, "Cannot remove '%s': Is a directory\n", dir);
			else
				_rm(dir);
		}
		ret = remove(dir);
		if (ret != 0)
			printfc(ERROR_COLOR, "Failed to remove '%s': %d\n", dir, ret);
		
		if (enableVerbose)
			printfc(MSG_COLOR, "Removed%s'%s'\n", is_dir ? " directory " : " ", dir);
	}
	if (size > 0)
		printfc(ERROR_COLOR, "Short read in directory '%s'\n", path);
	if (size < 0)
		printfc(ERROR_COLOR, "Error reading directory '%s': %d\n", path, size);
}
