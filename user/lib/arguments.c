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
#include <ctype.h>
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
			// opterr = ERRNO_INVALID_OPTION;
			// sprintf(_opt_buffer, "Illegal argument %c", opt);
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


/*
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
** Token
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

// const char ENTITIES[] = "<>();&|";
const char ENTITIES[] = "<>;&|";	// for now, we do not support brackets

static const char QUOTES[] = "\"\'";

static int _is_redirect_double(const char* p);
static token_t _get_token_type(char ch);
const char* get_token_str(token_t token);

static char* nextc;
static char cache = 0;

token_t get_token(char* str, char** token)
{
	if (str)
	{
		nextc = str;
		cache = 0;
	}

	if (cache)
	{
		if (token)
			*token = NULL;
		int ret = _get_token_type(cache);
		cache = 0;
		return ret;
	}

	// check emptiness
	if (!nextc || !*nextc)
	{
		if (token)
			*token = NULL;
		return TK_EMPTY;
	}

	// skip leading white spaces
	while (nextc && isspace(*nextc))
		nextc++;
	if (!nextc)
	{
		if (token)
			*token = NULL;
		return TK_EMPTY;;
	}

	// begin of a token is the return address
	char* begin = nextc;
	char quote = 0;		// whether in quote or not
	while (*nextc)
	{
		if (quote == 0)	// not in quote
		{
			if (strchr(QUOTES, *nextc))	// encounter quote
			{
				quote = *nextc;
				*nextc = '\0';
				nextc++;
				begin = nextc;			// begin also need to move forward
				continue;
			}
			if (strchr(ENTITIES, *nextc))
			{
				// handle case like 'cd; ls.b'
				if (begin == nextc)	// no previous
				{
					if (token)
						*token = NULL;	// if is token, this will be no-sense
					if (_is_redirect_double(nextc))
					{
						nextc += 2;
						return _get_token_type('}');
					}
					else
						return _get_token_type(*(nextc++));
				}
				else
				{
					if (_is_redirect_double(nextc))
					{
						cache = '}';
						*nextc = '\0';
						*(nextc + 1) = '\0';
						nextc += 2;
					}
					else
					{
						cache = *nextc;
						*(nextc++) = '\0';
					}
					if (token)
						*token = begin;
					return TK_WORD;
				}
			}
			if (isspace(*nextc))	// end of token
			{
				*(nextc++) = '\0';
				if (token)
					*token = begin;
				return TK_WORD;
			}

			// then *nextc is a regular character
			nextc++;
		}
		else	// in quotes
		{
			// Here, *nextc won't be '\0'.
			if (*nextc == quote)	// end of quote, then end of token
			{
				*(nextc++) = '\0';
				if (token)
					*token = begin;
				return TK_WORD;
			}
			nextc++;
		}
	}

	// If reach here, it means the quote is not closed, or simply not white
	// space after the last word.
	if (token)
		*token = begin;

	if (quote)	// quote not closed
	{
		// then the caller decide whether to use this token
		return TK_INVALID;
	}
	
	return TK_WORD;
}

static int _is_redirect_double(const char* p)
{
	return (*p == '>') && (*(p + 1) == '>');
}

static token_t _get_token_type(char ch)
{
	// const char ENTITIES[] = "<>();&|";
	switch (ch)
	{
	case '<':
		return TK_REDIRECT_LEFT;
	case '>':
		return TK_REDIRECT_RIGHT;
	case '}':
		// >> will be converted to '}'
		return TK_REDIRECT_DOUBLE;
	case '(':
		return TK_BRACKET_LEFT;
	case ')':
		return TK_BRACKET_RIGHT;
	case ';':
		return TK_SEMI_COLON;
	case '&':
		return TK_AMPERSAND;
	case '|':
		return TK_PIPE;
	default:
		return TK_INVALID;
	}
}

const char* get_token_str(token_t type)
{
	switch (type)
	{
	case TK_REDIRECT_LEFT:
		return "<";
	case TK_REDIRECT_RIGHT:
		return ">";
	case TK_REDIRECT_DOUBLE:
		return ">>";
	case TK_BRACKET_LEFT:
		return "(";
	case TK_BRACKET_RIGHT:
		return ")";
	case TK_SEMI_COLON:
		return ";";
	case TK_AMPERSAND:
		return "&";
	case TK_PIPE:
		return "|";
	default:
		return "?";
	}
}
