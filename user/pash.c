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

#include <lib.h>
#include <pash.h>
#include <conio.h>
#include <arguments.h>
#include <ctype.h>

static char buffer[PASH_BUFFER_SIZE];
static char username[64];
static char homeDir[MAXPATHLEN];
static char historyFile[MAXPATHLEN];

static int redirect;
static int backupfd[2];

static int trivial;	// indicate if is trivial for pipe

static int interactive;
static int echocmds;
static int verbose;
static char* filename;

static int showHelp;

static void init();

static void usage();
static int parse_args(int argc, char* argv[]);

static void print_splash();
static void print_prompt();

static int execute(char* cmd);
static int _runcmd(char* cmd);
static int _parsecmd(char* cmd, int* argc, char* argv[], int* rightpipe);
static int _execv(char* cmd, char* argv[]);
static void _restore_stream();


// History management
static int init_history();
static int append_history(const char* record);
static int get_history(int index, char* record);

static input_history_t history;

// Completion
int completer(const char* input, char* completion, int* revert);

// main function
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

	if (filename)
	{
		interactive = 0;
		close(0);
		if (open(filename, O_RDONLY) != 0)
		{
			PASH_ERR("Can't open '%s'\n", filename);
			return 2;
		}
	}

	panic_on(profile(username, homeDir, 0));
	strcpy(historyFile, homeDir);
	if (!is_ends_with(historyFile, "/"))
		strcat(historyFile, "/");
	strcat(historyFile, ".history");

	if (!interactive)
	{
		trivial = 1;
		backupfd[0] = dup(0, 3);
		panic_on(backupfd[0] < 0);
		backupfd[1] = dup(1, 4);
		panic_on(backupfd[1] < 0);
	}

	// init history, only enable when interactive
	init_input_history(&history);
	if (interactive)
	{
		history.init = init_history;
		history.get = get_history;
		history.append = append_history;
	}

	input_opt_t opt;
	init_input_opt(&opt);
	opt.minLen = 1;
	opt.maxLen = PASH_BUFFER_SIZE - 1;
	// opt.interruptible = 1;
	if (interactive && (history.count >= 0))
		opt.history = &history;
	opt.completer = completer;

	if (interactive)
		print_splash();

	int ret;
	for (; ; )
	{
		if (interactive)
			print_prompt();

		ret = get_string(buffer, &opt);
		putch('\n');

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
		if (ret != 0 && verbose)
			PASH_ERR("Command '%s' returned '%d'\n", buffer, ret);
	}

	return 0;
}

static void init()
{
	trivial = 0;
	interactive = iscons(0);

	echocmds = 0;
	verbose = 0;
	filename = NULL;
	showHelp = 0;
}

static void usage()
{
	PASH_MSG("Usage: pash [-ixvh] [command-file]\n");
}

static int parse_args(int argc, char* argv[])
{
	int opt;
	int arg_cnt = 0;
	int err = 0;
	while ((opt = getopt(argc, argv, "ixvh")))
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
		case 'v':
			verbose = 1;
			break;
		case 'h':
			showHelp = 1;
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

static void print_splash()
{
	// execli("clear", "clear", NULL);

	printfc(FOREGROUND_INTENSE(MAGENTA), " __________________________________________________________ \n");
	printfc(FOREGROUND_INTENSE(MAGENTA), "/                                                          \\\n");
	printfc(FOREGROUND_INTENSE(GREEN), "|                     Pash Host for MOS                    |\n");
	printfc(FOREGROUND_INTENSE(YELLOW), "|                                                          |\n");
	printfc(FOREGROUND_INTENSE(YELLOW), "|             Copyright (C) Tony's Studio 2023             |\n");
	printfc(FOREGROUND_INTENSE(YELLOW), "|                                                          |\n");
	printfc(FOREGROUND_INTENSE(WHITE), "|                  Based on PassBash v3.x                  |\n");
	printfc(FOREGROUND_INTENSE(MAGENTA), "\\__________________________________________________________/\n\n");

	execli("version", "version", NULL);

	printf("\n");
}

/********************************************************************
** I don't understand, why this pwd can't be declared as a static
** global variable, and placed in .bss or .data section. >:(
*/
static void print_prompt()
{
	char pwd[MAXPATHLEN];

	printfc(FOREGROUND_INTENSE(GREEN), "%s", username);
	printfc(FOREGROUND_INTENSE(WHITE), ":");
	getcwd(pwd);
	printfc(FOREGROUND_INTENSE(BLUE), "%s", pwd);
	printfc(FOREGROUND_INTENSE(WHITE), "$ ");
}

static int execute(char* cmd)
{
	int ret;

	strstrip(cmd, ' ');
	ret = _runcmd(cmd);

#ifdef MOS_VERBOSE
	if (ret != 0)
		PASH_ERR("Failed to run command '%s': %d\n", cmd, ret);
#endif

	return ret;
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
		redirect = 0;
		rightpipe = 0;

		// parse command
		ret = _parsecmd(cmd, &argc, argv, &rightpipe);
		if (ret < 0)
		{
			if (verbose)
				PASH_ERR(SYNTAX_ERR_MSG "Failed to parse command: %d\n", ret);
			_restore_stream();
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
			// PASH_MSG("Empty line...\n");
			_restore_stream();
			return 0;
		}

		// run new process
		int child = _execv(argv[0], argv);
		if (child < 0)
		{
			if (child == -E_NOT_FOUND)
				PASH_ERR("%s: Command not found\n", argv[0]);

			_restore_stream();
			if (hasNext)
				continue;
			else
				return child;
		}

		// why this has to be placed before wait?
		_restore_stream();

		if ((child > 0) && needWait)
			wait(child);

		if (rightpipe)
			wait(rightpipe);
	} while (hasNext);

	if (trivial)
		exit();

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
	const char* err = NULL;

	// initialize default value
	*argc = 0;
	argv[0] = NULL;
	*rightpipe = 0;

	// get first token
	char* token;
	token_t type = get_token(cmd, &token, &err);
	if (type == TK_EMPTY)
		return 0;
	if (type != TK_WORD)
	{
		// If first token is a separator, ignore it.
		if ((type != TK_SEMI_COLON) && (type != TK_AMPERSAND))
			PASH_ERR(SYNTAX_ERR_MSG "syntax error near unexpected token `%s'\n", get_token_str(type));
		return 0;
	}
	argv[(*argc)++] = token;	// command name

	// may be needed
	int fd;
	int pipefd[2];
	int ret;

	// get rest tokens
	for (; ; )
	{
		type = get_token(NULL, &token, &err);
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
			if ((type = get_token(NULL, &token, &err)) != TK_WORD)
			{
				PASH_ERR("Syntax error near unexpected token `%s'\n", get_token_str(type));
				PASH_MSG("`<' not followed by word\n");
				return -3;
			}
			if (isdir(token))
			{
				PASH_ERR("'%s': Is a directory\n", token);
				return -4;
			}

			fd = open(token, O_RDONLY);
			if (fd < 0)
			{
				PASH_ERR("Cannot open '%s' for reading\n", token);
				return -4;
			}
			dup(fd, 0);
			close(fd);

			redirect = 1;

			break;

		case TK_REDIRECT_RIGHT:
			if ((type = get_token(NULL, &token, &err)) != TK_WORD)
			{
				PASH_ERR("Syntax error near unexpected token `%s'\n", get_token_str(type));
				PASH_MSG("`>' not followed by word\n");
				return -3;
			}
			if (isdir(token))
			{
				PASH_ERR("'%s': Is a directory\n", token);
				return -4;
			}

			// create new file if not exists
			fd = open(token, O_WRONLY | O_CREAT);
			if (fd < 0)
			{
				PASH_ERR("Cannot open '%s' for writing\n", token);
				return -4;
			}
			dup(fd, 1);
			close(fd);

			redirect = 1;

			break;

		case TK_REDIRECT_DOUBLE:
			if ((type = get_token(NULL, &token, &err)) != TK_WORD)
			{
				PASH_ERR("Syntax error near unexpected token `%s'\n", get_token_str(type));
				PASH_MSG("`>>' not followed by word\n");
				return -3;
			}

			if (isdir(token))
			{
				PASH_ERR("'%s': Is a directory\n", token);
				return -4;
			}

			// create new file if not exists
			fd = open(token, O_WRONLY | O_CREAT | O_APPEND);
			if (fd < 0)
			{
				PASH_ERR("Cannot open '%s' for writing\n", token);
				return -4;
			}
			dup(fd, 1);
			close(fd);

			redirect = 1;

			break;

		case TK_PIPE:
			ret = pipe(pipefd);
			if (ret != 0)
			{
				PASH_ERR("Failed to create pipe\n");
				return -5;
			}

			redirect = 1;

			ret = fork();
			if (ret < 0)
			{
				PASH_ERR("Failed to fork\n");
				return -6;
			}
			*rightpipe = ret;
			if (ret == 0)
			{
				trivial = 1;
				dup(pipefd[0], 0);
				close(pipefd[0]);
				close(pipefd[1]);
				// NULL to continue parsing on current cmd string
				return _parsecmd(NULL, argc, argv, rightpipe);
			}
			else
			{
				dup(pipefd[1], 1);
				close(pipefd[0]);
				close(pipefd[1]);
				return 0;
			}
			break;

		case TK_SEMI_COLON:
			// printf("TK_SEMI_COLON\n");
			return 1;

		case TK_AMPERSAND:
			// printf("TK_AMPERSAND\n");
			return 2;

		default:
			PASH_ERR("Syntax error: %s\n", err ? err : "unknown token");
			return -66;
		}	// switch (type)
	}

	return 0;
}

static int _execv(char* cmd, char* argv[])
{
	int ret = execvi(cmd, argv);
	if (ret != -1)	// execute success or failed
		return 0;

	return execv(cmd, argv);
}

static void _restore_stream()
{
	if (!redirect)
		return;

	// debugf("restore %d", interactive);

	if (!interactive)
	{
		dup(backupfd[0], 0);
		dup(backupfd[1], 1);
	}
	else
	{
		close(0);
		panic_on(opencons() != 0);
		panic_on(dup(0, 1) < 0);
	}
}

/*
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
** History
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
static int _get_history_count(int fd)
{
	int ret = 0;
	char buf;

	int offset = ftell(fd);
	if (offset < 0)
		return 0;

	seek(fd, 0);
	while (read(fd, &buf, 1) == 1)
	{
		if (buf == '\n')
			ret++;
	}
	seek(fd, (u_int)offset);

	return ret;
}

static int init_history()
{
	int fd = open(historyFile, O_RDONLY | O_CREAT);
	if (fd < 0)
		return fd;

	history.count = _get_history_count(fd);

	close(fd);

	return 0;
}

static int append_history(const char* record)
{
	int fd = open(historyFile, O_WRONLY | O_CREAT | O_APPEND);
	if (fd < 0)
		return fd;

	write(fd, record, strlen(record));
	write(fd, "\n", 1);

	close(fd);

	history.count++;

	return 0;
}

static int get_history(int index, char* record)
{
	int fd = open(historyFile, O_RDONLY | O_CREAT);
	if (fd < 0)
		return fd;

	int count = 0;
	while (readline(fd, record) > 0)
	{
		if (count == index)
		{
			close(fd);
			strstripr(record, '\n');
			return 0;
		}
		count++;
	}

	close(fd);

	// reset history
	history.count = count;

	// not found
	return -1;
}


/*
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
** Completion
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
#define MAX_CANDIDATES 32

static char _candidates[MAX_CANDIDATES][MAXNAMELEN];

static void _get_candidates(const char* path)
{
	_candidates[0][0] = '\0';	// clear candidates

	int fd = open(path, O_RDONLY);
	if (fd < 0)
		return;

	struct Stat st;
	if (fstat(fd, &st) < 0)
		return;
	if (!st.st_isdir)
		return;

	int size;
	struct File file;
	char(*candidate)[MAXNAMELEN] = _candidates;
	while ((size = readn(fd, &file, sizeof(struct File))) == sizeof(struct File))
	{
		if (file.f_name[0])
		{
			strcpy(*candidate, file.f_name);
			candidate++;
		}
	}
	(*candidate)[0] = '\0';

	close(fd);
}

// This tricky one comes from PassBash... I don't know how I
// came up with this before. :P
static const char* _begins_with(const char* str, const char* prefix)
{
	if (is_null_or_empty(str) || is_null_or_empty(prefix))
		return str;

	const char* original_str = str;
	while (*prefix && *str)
	{
		if (*prefix != *str)
			return original_str;
		prefix++;
		str++;
	}

	return str;
}

/********************************************************************
** Return:
**   0: no match
**   1: match
*/
int completer(const char* input, char* completion, int* revert)
{
	*completion = '\0';

	if (strstr(input, "//"))	// bad path
		return 0;

	// skip leading white spaces
	while (*input && isspace(*input))
		input++;

	/*
	if (!*input)
		return 0;
	*/

	// Get last substring to complete.
	const char* last = input;
	while (*last)
	{
		while (*last && !isspace(*last))
			last++;
		while (*last && isspace(*last))
			last++;
		if (*last)
			input = last;
	}

	char parent[MAXPATHLEN];
	char name[MAXNAMELEN];

	name[0] = '\0';
	if (input[0] == '\0')
		strcpy(parent, "./");
	else if (is_begins_with(input, "~/"))
	{
		strcpy(parent, homeDir);
		if (!is_ends_with(input, "/"))
			basename(input, name);
	}
	else if (is_ends_with(input, "/"))	// empty path
		strcpy(parent, input);
	else
	{
		parentpath(input, parent);
		basename(input, name);
	}

	// debugf("[P: %s - B: %s]", parent, name);

	_get_candidates(parent);
	if (_candidates[0][0] == '\0')
		return 0;

	const char* match = NULL;
	char(*candidate)[MAXNAMELEN] = _candidates;

	/*
	while ((*candidate)[0] != '\0')
	{
		debugf("<%s>", *candidate);
		candidate++;
	}
	candidate = _candidates;
	*/

	if (revert)
		*revert = 0;
	if (name[0] == '\0')	// empty path will take first feasible candidate
	{
		match = *candidate;
		strcpy(completion, match);
		return 1;
	}

	while ((*candidate)[0] != '\0')
	{
		const char* pos = _begins_with(*candidate, name);
		if (!*pos)	// full match
		{
			candidate++;
			match = (*candidate)[0] ? *candidate : _candidates[0];
			if (revert)
				*revert = (int)strlen(name);
			break;
		}
		if (pos != *candidate)	// partial match
		{
			if (!match)	        // accept first match
			{
				match = pos;
				break;
			}
			else
				return 0;	// ignore multiple possible matches
		}

		candidate++;
	}

	if (!match)
		return 0;

	strcpy(completion, match);

	return 1;
}
