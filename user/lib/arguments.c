/********************************************************************
** arguments.c
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
** This file provides replica to Unix getopt() function.
**
** Based on PassBash.
**   - https://github.com/Lord-Turmoil/PassBashPro
**   - https://gitee.com/tonys-studio/pass-bash-pro
*/


#include <lib.h>
#include <arguments.h>

static char _opt_buffer[128];

char* optarg;
const char* optmsg;
int opterr;
int optopt;

static int optind;

void resetopt()
{
	optind = 0;
	opterr = 0;
	optarg = NULL;
	optmsg = NULL;
}

static int _parseopt(const char* arg);

// parse arg in opt. e.g. pthread from -lpthread
// Must checked by _parseopt first. 
static char* _parsearg(char* arg);

static void _initopt();

int getopt(int argc, char* argv[], const char* pattern)
{
	_initopt();

	if (optind >= argc)
	{
		resetopt();
		return 0;
	}

	int opt = _parseopt(argv[optind]);
	if (!opt)	// not an option
	{
		opt = optopt = '!';
		optarg = argv[optind];
	}
	else		// is an option
	{
		optopt = opt;
		const char* pos = strchr(pattern, opt);
		if (!pos)	// not valid option
		{
			opt = '?';
			opterr = ERRNO_INVALID_OPTION;
			sprintf(_opt_buffer, "Illegal argument %c", opt);
		}
		else
		{
			if (pos[1] == ':')	// has argument
			{
				optarg = _parsearg(argv[optind]);
				if (!optarg)	// argument not compact
				{
					if (((optind < argc - 1) && _parseopt(argv[optind + 1])) || \
						(optind == argc - 1))
					{
						opterr = ERRNO_MISSING_ARGUMENT;
						sprintf(_opt_buffer,
								  "Missing argument for parameter %c", opt);
						optmsg = _opt_buffer;
					}
					else
						optarg = argv[++optind];
				}
			}
		}
	}

	optmsg = _opt_buffer;

	return opt;
}

static int _parseopt(const char* arg)
{
	if (arg[0] == '-' && arg[1] != '\0')
		return arg[1];
	else
		return 0;
}

// parse arg in opt. e.g. pthread from -lpthread
// Must checked by _parseopt first. 
static char* _parsearg(char* arg)
{
	if (arg[2] != '\0')
		return &arg[2];
	else
		return NULL;
}

static void _initopt()
{
	optarg = NULL;
	optmsg = NULL;
	opterr = 0;
	optopt = '?';
	optind++;
}