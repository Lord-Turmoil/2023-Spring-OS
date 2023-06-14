/********************************************************************
** history.c
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
**   This file implements history command.
*/

#include <lib.h>
#include <arguments.h>

static const char HISTORY_FILE[] = "/home/tony/.history";

static int enableClear;
static int showHelp;

static void usage();
static int parse_args(int argc, char* argv[]);

static int clear_history();
static int show_history();

int main(int argc, char* argv[])
{
	enableClear = 0;

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

	if (enableClear)
	{
		if (clear_history() == 0)
			printfc(MSG_COLOR, "Command history cleared\n");
		else
			printfc(ERROR_COLOR, "Failed to clear history\n");
	}
	else
		show_history();
}

static void usage()
{
	printfc(MSG_COLOR, "Usage: history [-ch]\n");
}

static int parse_args(int argc, char* argv[])
{
	int opt;
	int err = 0;
	while ((opt = getopt(argc, argv, "ch")))
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
		case 'c':
			enableClear = 1;
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

static int clear_history()
{
	int fd = open(HISTORY_FILE, O_WRONLY | O_CREAT);
	if (fd < 0)
		return fd;
	
	close(fd);
	
	return 0;
}

static int show_history()
{
	int fd = open(HISTORY_FILE, O_RDONLY);
	if (fd < 0)
		return fd;

	int count = 0;
	char buffer[1024];

	while (readline(fd, buffer) > 0)
	{
		strstripr(buffer, '\n');
		printf("%10d  %s\n", ++count, buffer);
	}

	return 0;
}
