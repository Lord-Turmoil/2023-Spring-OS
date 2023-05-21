/********************************************************************
** pash.c
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
** Main file for Pash.
**
** Based on PassBash.
**   - https://github.com/Lord-Turmoil/PassBashPro
**   - https://gitee.com/tonys-studio/pass-bash-pro
*/

#include <pash.h>
#include <conio.h>
#include <arguments.h>

static char buffer[PASH_BUFFER_SIZE];

static int interactive;
static int echocmds;
static char* filename;

static void _usage(void);
static int _parse_args(int argc, char* argv[]);

static int execute(char* cmd);
static int excv();

int main(int argc, char* argv[])
{
	interactive = iscons(0);
	echocmds = 1;
	filename = NULL;

	if (_parse_args(argc, argv) != 0)
	{
		_usage();
		return 1;
	}
	
	input_opt_t opt;
	init_input_opt(&opt);
	opt.minLen = 1;
	opt.maxLen = PASH_BUFFER_SIZE - 1;
	opt.interruptible = 1;
	
	printf("Pash Host for MOS v0.1.0\n");
	printf("    Copyright (C) Tony's Studio 2023\n");
	printf("Based on PassBash v3.x\n");
	printf("__________________________________________________\n\n");


	int ret;
	for (; ; )
	{
		if (interactive)
			printf("\nPASH HOST $ ");
		ret = get_string(buffer, &opt);
		printf("\n");
		if (ret == EOF)
		{
			debugf("End-of-File reached!\n");
			return 0;
		}
		if (ret < opt.minLen)	// interrupted
			continue;
		if (is_no_content(buffer))	// empty input
			continue;

		if (echocmds)
			printf("#> %s\n", buffer);

		/*ret = fork();
		if (ret < 0)
			user_panic("fork() returned %d", ret);
		if (ret == 0)
			return execute(buffer);
		else
			wait(ret);*/
	}

	return 0;
}

static void _usage(void)
{
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}

static int _parse_args(int argc, char* argv[])
{
	int opt;
	int arg_cnt = 0;
	int err = 0;
	while (opt = getopt(argc, argv, "ix"))
	{
		if (opterr != 0)
		{
			debugf("Argument error: %s\n", optmsg);
			err = 1;
			resetopt();
			break;
		}
		switch (opt)
		{
		case 'i':
			interactive = 1;
			break;
		case 'x':
			echocmds = 1;
			break;
		case '!':
			arg_cnt++;
			if (arg_cnt == 1)
				filename = optarg;
			else if (arg_cnt == 2)
			{
				err = 1;
				debugf(ERRMSG_TOO_MANY "\n");
			}
			break;
		case '?':
			err = 1;
			debugf("Unknown parameter \"-%c\"\n", optopt);
			break;
		default:
			break;
		}
	}

	if (err)
	{
		debugf(ERRMSG_ILLEGAL "\n");
		return 1;
	}

	return 0;
}

static int execute(char* cmd);
static int excv();