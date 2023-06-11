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

static void _print_prompt();

static int execute(char* cmd);
static int _runcmd(char* cmd);
static int _parsecmd(char* cmd, int* argc, char* argv[], int* rightpipe);

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

	printf("\n__________________________________________________\n\n");
	printf("Pash Host for MOS\n\n");
	printf("    Copyright (C) Tony's Studio 2023\n\n");
	printf("Based on PassBash v3.x\n");
	printf("__________________________________________________\n\n");

	execute_internal("version");

	printf("\n");

	int ret;
	for (; ; )
	{
		if (interactive)
			_print_prompt();

		ret = get_string(buffer, &opt);
		printf("\n");
		if (ret == EOF)
		{
			PASH_MSG("End-of-File reached!\n");
			return 0;
		}
		if (ret < opt.minLen)	// interrupted
			continue;
		if (is_no_content(buffer))	// empty input
			continue;

		if (echocmds)
			printf("#> %s\n", buffer);

		ret = execute(buffer);
		if (ret != 0)
			printfc(FOREGROUND(RED), "Command '%s' returned '%d'\n", buffer, ret);
	}

	return 0;
}

static void _usage(void)
{
	PASH_MSG("usage: sh [-dix] [command-file]\n");
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
			PASH_ERR("Argument error: %s\n", optmsg);
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
				PASH_ERR(ERRMSG_TOO_MANY "\n");
			}
			break;
		case '?':
			err = 1;
			PASH_ERR("Unknown parameter \"-%c\"\n", optopt);
			break;
		default:
			break;
		}
	}

	if (err)
	{
		PASH_ERR(ERRMSG_ILLEGAL "\n");
		return 1;
	}

	return 0;
}

static void _print_prompt()
{
	printfc(FOREGROUND_INTENSE(GREEN), "tony");
	printfc(FOREGROUND(WHITE), ":");
	printfc(FOREGROUND_INTENSE(BLUE), "/path");
	printfc(FOREGROUND(WHITE), "$ ");
}

static int execute(char* cmd)
{
	int ret;

	strstrip(cmd, ' ');

	ret = execute_internal(cmd);
	if (ret >= 0)
		return ret;

	// execute external commands
	ret = fork();
	if (ret < 0)
	{
		PASH_ERR("Failed to execute due to fork: %d\n", ret);
		return ret;
	}

	if (ret == 0)
	{
		ret = _runcmd(cmd);
		if (ret != 0)
			PASH_ERR("Command returned: %d\n", ret);
		exit();
	}
	else
		wait(ret);

	return 0;
}

static int _runcmd(char* cmd)
{
	int argc;
	char* argv[PASH_MAXARGS];
	int rightpipe = 0;

	int ret;
	int hasNext;
	int needWait;

	do
	{
		// initialize default behavior
		hasNext = 0;
		needWait = 1;

		// parse command
		ret = _parsecmd(cmd, &argc, argv, &rightpipe);
		if (ret < 0)
		{
			PASH_ERR(SYNTAX_ERR_MSG "Failed to parse command: %d\n", ret);
			return ret;
		}
		if (ret > 0)	// ; or &
		{
			hasNext = 1;
			if (ret == 2)	// &
				needWait = 0;
			cmd = NULL;	// continue on previous cmd
		}
		argv[argc] = NULL;

		// no command
		if (argc == 0)
		{
			PASH_MSG("Empty line...\n");
			return 0;
		}

		// run new process
		int child = spawn(argv[0], argv);
		if (child < 0)
		{
			PASH_ERR("Failed to spawn '%s'\n", argv[0]);
			if (hasNext)
				continue;
			else
				return child;
		}
		
		// If close all, later children will be unable to output...
		// close_all();

		if (needWait)
			wait(child);

		if (rightpipe)
			wait(rightpipe);
	} while (hasNext);

	return 0;
}

/********************************************************************
** Return value:
**  < 0: error encountered
**  = 0: good to go
**  = 1: ';' encountered, wait, and parse again
**  = 2: '&' encountered, no wait, and parse again
*/
static int _parsecmd(char* cmd, int* argc, char* argv[], int* rightpipe)
{
	// initialize default value
	*argc = 0;
	argv[0] = NULL;
	*rightpipe = 0;

	// get first token
	char* token;
	token_t type = get_token(cmd, &token);
	if (type == TK_EMPTY)
		return 0;
	if (type != TK_WORD)
	{
		PASH_ERR(SYNTAX_ERR_MSG "Command not begin with word\n");
		return -1;
	}
	argv[(*argc)++] = token;	// command name

	// may be needed
	int fd;
	int pipefd[2];
	int ret;

	// get rest tokens
	for (; ; )
	{
		type = get_token(NULL, &token);
		if (type == TK_EMPTY)
			return 0;
		switch (type)
		{
		case TK_WORD:
			if (*argc >= PASH_MAXARGS)
			{
				PASH_ERR(ARGUMENT_ERR_MSG "Too many arguments\n");
				return -2;
			}
			argv[(*argc)++] = token;
			break;
		case TK_REDIRECT_LEFT:
			if (get_token(NULL, &token) != TK_WORD)
			{
				PASH_ERR(SYNTAX_ERR_MSG "'<' not followed by word\n");
				return -3;
			}
			fd = open(token, O_RDONLY);
			if (fd < 0)
			{
				PASH_ERR("Cannot open '%s' for reading\n", token);
				return -4;
			}
			dup(fd, 0);
			close(fd);
			break;
		case TK_REDIRECT_RIGHT:
			if (get_token(NULL, &token) != TK_WORD)
			{
				PASH_ERR(SYNTAX_ERR_MSG "'>' not followed by word\n");
				return -5;
			}
			fd = open(token, O_WRONLY);
			if (fd < 0)
			{
				PASH_ERR("Cannot open '%s' for writing\n", token);
				return -6;
			}
			dup(fd, 1);
			close(fd);
			break;
		case TK_PIPE:
			ret = pipe(pipefd);
			if (ret != 0)
			{
				PASH_ERR("Failed to create pipe\n");
				return -7;
			}
			ret = fork();
			if (ret < 0)
			{
				PASH_ERR("Failed to fork\n");
				return -8;
			}
			*rightpipe = ret;
			if (ret == 0)
			{
				dup(pipefd[0], 0);
				close(pipefd[0]);
				close(pipefd[1]);
				// NULL to continue parsing on current cmd strinrg
				return _parsecmd(NULL, argc, argv, rightpipe);
			}
			else
			{
				dup(pipefd[1], 1);
				close(pipefd[0]);
				close(pipefd[1]);
				return 0;
			}
		case TK_SEMI_COLON:
			// printf("TK_SEMI_COLON\n");
			return 1;
		case TK_AMPERSAND:
			// printf("TK_AMPERSAND\n");
			return 2;
		default:
			PASH_ERR("Unknown token\n");
			return -66;
		}	// switch (type)
	}

	return 0;
}