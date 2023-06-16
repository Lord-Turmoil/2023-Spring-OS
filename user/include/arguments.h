/********************************************************************
** arguments.h
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
**   This file provides replica to Unix getopt() function. And a
** shabby tokenizer.
** 
**
** Based on PassBash.
**   - https://github.com/Lord-Turmoil/PassBashPro
**   - https://gitee.com/tonys-studio/pass-bash-pro
*/

#ifndef _ARGUMENTS_H_
#define _ARGUMENTS_H_

extern char* optarg;
extern const char* optmsg;
extern int opterr;
extern int optopt;

void resetopt();
int getopt(int argc, char* argv[], const char* pattern);

#define ERRNO_INVALID_OPTION   1
#define ERRNO_MISSING_ARGUMENT 2

#define ERRMSG_TOO_MANY "Too many arguments!"
#define ERRMSG_TOO_FEW  "Too few arguments!"
#define ERRMSG_ILLEGAL  "Arguments illegal!"

extern const char ENTITIES[];
typedef enum _token_t
{
	TK_INVALID = 0,
	TK_EMPTY,

	TK_REDIRECT_LEFT,
	TK_REDIRECT_RIGHT,
	TK_REDIRECT_DOUBLE,

	TK_BRACKET_LEFT,
	TK_BRACKET_RIGHT,

	TK_SEMI_COLON,
	TK_AMPERSAND,

	TK_PIPE,

	TK_WORD,

	TK_COUNT
} token_t;

const char* get_token_str(token_t token);
token_t get_token(char* str, char** token, const char** err);

#endif
