/********************************************************************
** pash.h
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
** This file declares global variables and functions for Pash host.
**
** Based on PassBash.
**   - https://github.com/Lord-Turmoil/PassBashPro
**   - https://gitee.com/tonys-studio/pass-bash-pro
*/

#ifndef _PASH_H_
#define _PASH_H_

#include <lib.h>

/*
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
** ASCII Constants
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
#define LINEFEED '\n'
#define CARRIGE  '\r'

#define SPACE ' '
#define TAB   '\t'

#define BACKSPACE      127
#define CTRL_BACKSPACE 8

// Unfortunately, this is also the special leading in MOS. :(
#define ESCAPE 27

/*
 * Special key consists of... 3+ characters. The first two are leading.
 * For example, in Bash, you can actually type in ESC, '[', 'A' in a row
 * to simulate a Arrow Up key... :(
 */
#define SPECIAL_LEADING 27

#define SPECIAL_ARROW_UP    65
#define SPECIAL_ARROW_DOWN  66
#define SPECIAL_ARROW_LEFT  68
#define SPECIAL_ARROW_RIGHT 67

#define SPECIAL_HOME 72
#define SPECIAL_END  70

#define SPECIAL_DELETE 126

#define CTRL_D 0x04

 /*
 **+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 ** Input History
 **+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

 /*
  * In Linux, it seems that input history will only be saved to .history
  * on exit. So they still need to be kept in memory. But here, we don't
  * have dynamic memory allocation, which makes this almost impossible.
  */

  // ON HOLD

  /*
  **+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  ** Input Options & Context
  **+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  */

  /*
   * These are inherited from PassBash, with signatures to better match C style.
   * Fancy abilities are removed or simplified due to the you-know-what limits.
   */

typedef struct _input_opt_t
{
	int minLen;
	int maxLen;

	int interruptible;
} input_opt_t;

void init_input_opt(input_opt_t* opt);
void copy_input_opt(input_opt_t* dst, const input_opt_t* src);


typedef struct _input_ctx_t
{
	char* buffer;	// where the display string stored
	int pos;        // current cursor position
	int length;     // current input length

	int ch;        // next character to put into buffer
} input_ctx_t;

void init_input_ctx(input_ctx_t* ctx);
void copy_input_ctx(input_ctx_t* dst, const input_ctx_t* src);

#define PASH_BUFFER_SIZE 1024

/*
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
** Core Functions
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

// Ahh... C does not support override... :'(
int get_string(char* buffer, const input_opt_t* options);


/*
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
** Auxiliary Functions
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
int is_terminator(int ch);
int is_null_or_empty(const char* str);
int is_no_content(const char* str);

int is_the_same(const char* str1, const char* str2);

int is_begins_with(const char* str, const char* prefix);


/*
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
** Command Execution
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
#define PASH_MAXARGS 64

int execute_internal(char* cmd);


/*
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
** Errors
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

#define SYNTAX_ERR_MSG "Syntax Error: "
#define ARGUMENT_ERR_MSG "Argument Error: "

#define MSG_COLOR   FOREGROUND(YELLOW)
#define ERROR_COLOR FOREGROUND_INTENSE(RED)

#define PASH_MSG(...) printfc(MSG_COLOR, __VA_ARGS__)
#define PASH_ERR(...) printfc(ERROR_COLOR, __VA_ARGS__)

#endif
