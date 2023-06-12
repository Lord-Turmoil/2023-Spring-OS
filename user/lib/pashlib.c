/********************************************************************
** pashlib.c
**
**     Copyright (C) Tony's Studio. All rights reserved.
**
**   This file defines library functions for Pash. The original
** implementation comes from PassBash.
**   - https://github.com/Lord-Turmoil/PassBashPro
**   - https://gitee.com/tonys-studio/pass-bash-pro
*/

#include <pash.h>
#include <conio.h>
#include <ctype.h>

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

/*
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
** Input Options & Context
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
void init_input_opt(input_opt_t* opt)
{
	opt->maxLen = 1023;
	opt->minLen = 1;
	opt->interruptible = 0;
}

void copy_input_opt(input_opt_t* dst, const input_opt_t* src)
{
	if (dst && src)
		*dst = *src;
}

void init_input_ctx(input_ctx_t* ctx)
{
	ctx->buffer = NULL;
	ctx->length = 0;
	ctx->pos = 0;
	ctx->ch = 0;
}

void copy_input_ctx(input_ctx_t* dst, const input_ctx_t* src)
{
	if (dst && src)
		*dst = *src;
}

/*
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
** Core Functions
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
static void _insert_char(int ch);	// insert a character at current caret
static void _insert_n_char(int ch, int n);
static void _insert_space();		// insert a space at current caret
static void _insert_n_space(int n);
static void _insert_backspace();	// move caret left by one character
static void _insert_n_backspace(int n);
static void _insert_delete();		// delete character before caret
static void _insert_n_delete(int n);

typedef void _input_action_t(const input_opt_t* opt, input_ctx_t* ctx);
typedef int _input_handler_t(const input_opt_t* opt, input_ctx_t* ctx);

// basic input action
static _input_action_t _input_char;
static _input_action_t _input_backspace;

static _input_action_t _input_arrow_left;
static _input_action_t _input_arrow_right;
static _input_action_t _input_arrow_up;
static _input_action_t _input_arrow_down;

static _input_action_t _input_delete;

static _input_action_t _input_home;
static _input_action_t _input_end;

// control input action
static _input_action_t _input_ctrl_backspace;

static _input_action_t _input_arrow_ctrl_left;
static _input_action_t _input_arrow_ctrl_right;

static _input_action_t _input_ctrl_delete;

// input infinite machine node
// Ahhh.... NAMING!!!
static _input_handler_t _input_handler;
static _input_handler_t _special_root_handler;
static _input_handler_t _special_direct_handler;

static _input_handler_t _special_extend_handler;

static _input_handler_t _special_ctrl_root_handler;
static _input_handler_t _special_ctrl_inter_handler;
static _input_handler_t _special_ctrl_direct_handler;


int get_string(char* buffer, const input_opt_t* options)
{
	// initialize options and context
	input_opt_t opt;
	input_ctx_t ctx;

	copy_input_opt(&opt, options);
	opt.minLen = max(opt.minLen, 0);
	opt.maxLen = min(opt.maxLen, PASH_BUFFER_SIZE - 1);

	init_input_ctx(&ctx);
	ctx.buffer = buffer;
	ctx.buffer[0] = '\0';

	// Get string, huh?
	int ret;
	for (; ; )
	{
		ret = _input_handler(&opt, &ctx);
		if (ret == 0)
			continue;
		else if (ret == EOF)	// EOF
			return EOF;
		else if (ret < 0)	// interrupted
			return ctx.length;
		else				// ends normally
			break;
	}

	// record history

	return ctx.length;
}

// insert actions
static void _insert_char(int ch)
{
	if (isprint(ch))
		putch(ch);
}

static void _insert_n_char(int ch, int n)
{
	for (int i = 0; i < n; i++)
		_insert_char(ch);
}

static void _insert_space()
{
	_insert_char(' ');
}

static void _insert_n_space(int n)
{
	for (int i = 0; i < n; i++)
		_insert_space();
}

static void _insert_backspace()
{
	printf("\033[1D");
}

static void _insert_n_backspace(int n)
{
	if (n > 0)
		printf("\033[%dD", n);
}

static void _insert_delete()
{
	_insert_backspace();
	_insert_space();
	_insert_backspace();
}

static void _insert_n_delete(int n)
{
	for (int i = 0; i < n; i++)
		_insert_delete();
}

// input actions
void _input_char(const input_opt_t* opt, input_ctx_t* ctx)
{
	if (!((ctx->length < opt->maxLen) && isprint(ctx->ch)))
		return;

	for (int i = ctx->length; i > ctx->pos; i--)
		ctx->buffer[i] = ctx->buffer[i - 1];
	ctx->buffer[ctx->pos] = ctx->ch;
	ctx->length++;
	ctx->buffer[ctx->length] = '\0';
	for (int i = ctx->pos; i < ctx->length; i++)
		_insert_char(ctx->buffer[i]);
	ctx->pos++;
	_insert_n_backspace(ctx->length - ctx->pos);
}


// input actions
void _input_backspace(const input_opt_t* opt, input_ctx_t* ctx)
{
	if (ctx->pos <= 0)
		return;

	_insert_delete();
	for (int i = ctx->pos; i < ctx->length; i++)
	{
		_insert_char(ctx->buffer[i]);
		ctx->buffer[i - 1] = ctx->buffer[i];
	}
	_insert_space();
	_insert_n_backspace(ctx->length - ctx->pos + 1);
	ctx->pos--;
	ctx->length--;
	ctx->buffer[ctx->length] = '\0';
}

void _input_arrow_left(const input_opt_t* opt, input_ctx_t* ctx)
{
	if (ctx->pos > 0)
	{
		//_insert_backspace();
		printf("\033[1D");
		ctx->pos--;
	}
}

void _input_arrow_right(const input_opt_t* opt, input_ctx_t* ctx)
{
	if (ctx->pos < ctx->length)
	{
		//_insert_char(ctx->buffer[ctx->pos]);
		printf("\033[1C");
		ctx->pos++;
	}
}

void _input_arrow_up(const input_opt_t* opt, input_ctx_t* ctx)
{
}

void _input_arrow_down(const input_opt_t* opt, input_ctx_t* ctx)
{
}

void _input_delete(const input_opt_t* opt, input_ctx_t* ctx)
{
	if (ctx->pos >= ctx->length)
		return;
	for (int i = ctx->pos + 1; i < ctx->length; i++)
	{
		ctx->buffer[i - 1] = ctx->buffer[i];
		_insert_char(ctx->buffer[i]);
	}
	_insert_space();
	_insert_n_backspace(ctx->length - ctx->pos);
	ctx->length--;
	ctx->buffer[ctx->length] = '\0';
}

void _input_home(const input_opt_t* opt, input_ctx_t* ctx)
{
	_insert_n_backspace(ctx->pos);
	ctx->pos = 0;
}

void _input_end(const input_opt_t* opt, input_ctx_t* ctx)
{
	for (int i = ctx->pos; i < ctx->length; i++)
		_insert_char(ctx->buffer[i]);

	ctx->pos = ctx->length;
}

static void _input_ctrl_backspace(const input_opt_t* opt, input_ctx_t* ctx)
{
	while ((ctx->pos > 0) && !isalnum(ctx->buffer[ctx->pos - 1]))
		_input_backspace(opt, ctx);
	while ((ctx->pos > 0) && isalnum(ctx->buffer[ctx->pos - 1]))
		_input_backspace(opt, ctx);
}

static void _input_arrow_ctrl_left(const input_opt_t* opt, input_ctx_t* ctx)
{
	while ((ctx->pos > 0) && !isalnum(ctx->buffer[ctx->pos - 1]))
	{
		_insert_backspace();
		ctx->pos--;
	}
	while ((ctx->pos > 0) && isalnum(ctx->buffer[ctx->pos - 1]))
	{
		_insert_backspace();
		ctx->pos--;
	}
}

static void _input_arrow_ctrl_right(const input_opt_t* opt, input_ctx_t* ctx)
{
	while ((ctx->pos < ctx->length) && isalnum(ctx->buffer[ctx->pos]))
	{
		_insert_char(ctx->buffer[ctx->pos]);
		ctx->pos++;
	}
	while ((ctx->pos < ctx->length) && !isalnum(ctx->buffer[ctx->pos]))
	{
		_insert_char(ctx->buffer[ctx->pos]);
		ctx->pos++;
	}
}

static void _input_ctrl_delete(const input_opt_t* opt, input_ctx_t* ctx)
{
	if (isalnum(ctx->buffer[ctx->pos]))
	{
		while ((ctx->pos < ctx->length) && isalnum(ctx->buffer[ctx->pos]))
			_input_delete(opt, ctx);
	}
	while ((ctx->pos < ctx->length) && !isalnum(ctx->buffer[ctx->pos]))
		_input_delete(opt, ctx);
}


// special handlers
int _input_handler(const input_opt_t* opt, input_ctx_t* ctx)
{
	int ch = getch();

	if (ch == EOF)
		return EOF;

	if (is_terminator(ch))
	{
		if (ctx->length >= opt->minLen)
			return ctx->length;	// good exit
		else if (opt->interruptible)
			return -1;			// bad exit
		else
			return 0;			// continue
	}

	switch (ch)
	{
	case SPECIAL_LEADING:
		return _special_root_handler(opt, ctx);
	case BACKSPACE:
		_input_backspace(opt, ctx);
		break;
	case CTRL_BACKSPACE:
		_input_ctrl_backspace(opt, ctx);
		break;
	case TAB:
		// do completion
		break;
	default:
		if (ctx->length < opt->maxLen)
		{
			ctx->ch = ch;
			_input_char(opt, ctx);
		}
		break;
	}

	return 0;
}

int _special_root_handler(const input_opt_t* opt, input_ctx_t* ctx)
{
	int ch = getch();
	switch (ch)
	{
	case 91:
		return _special_direct_handler(opt, ctx);
	case EOF:
		return EOF;
	default:
		return 0;
	}
}

int _special_direct_handler(const input_opt_t* opt, input_ctx_t* ctx)
{
	int ch = getch();
	switch (ch)
	{
	case SPECIAL_ARROW_UP:
		_input_arrow_up(opt, ctx);
		break;
	case SPECIAL_ARROW_DOWN:
		_input_arrow_down(opt, ctx);
		break;
	case SPECIAL_ARROW_LEFT:
		_input_arrow_left(opt, ctx);
		break;
	case SPECIAL_ARROW_RIGHT:
		_input_arrow_ctrl_right(opt, ctx);
		break;
	case SPECIAL_HOME:
		_input_home(opt, ctx);
		break;
	case SPECIAL_END:
		_input_end(opt, ctx);
		break;
	case 49:
		return _special_ctrl_root_handler(opt, ctx);
	case 51:
		return _special_extend_handler(opt, ctx);
	case EOF:
		return EOF;
	default:
		return 0;
	}

	return 0;
}

int _special_extend_handler(const input_opt_t* opt, input_ctx_t* ctx)
{
	int ch = getch();

	switch (ch)
	{
	case SPECIAL_DELETE:
		_input_delete(opt, ctx);
		break;
	case 59:
		return _special_ctrl_inter_handler(opt, ctx);
	case EOF:
		return EOF;
	default:
		break;
	}

	return 0;
}

int _special_ctrl_root_handler(const input_opt_t* opt, input_ctx_t* ctx)
{
	int ch = getch();

	switch (ch)
	{
	case 59:
		return _special_ctrl_inter_handler(opt, ctx);
	case EOF:
		return EOF;
	default:
		break;
	}

	return 0;
}

int _special_ctrl_inter_handler(const input_opt_t* opt, input_ctx_t* ctx)
{
	int ch = getch();

	switch (ch)
	{
	case 53:
		return _special_ctrl_direct_handler(opt, ctx);
	case EOF:
		return EOF;
	default:
		break;
	}

	return 0;
}

int _special_ctrl_direct_handler(const input_opt_t* opt, input_ctx_t* ctx)
{
	int ch = getch();

	switch (ch)
	{
	case SPECIAL_ARROW_UP:
		_input_arrow_up(opt, ctx);
		break;
	case SPECIAL_ARROW_DOWN:
		_input_arrow_down(opt, ctx);
		break;
	case SPECIAL_ARROW_LEFT:
		_input_arrow_ctrl_left(opt, ctx);
		break;
	case SPECIAL_ARROW_RIGHT:
		_input_arrow_ctrl_right(opt, ctx);
		break;
	case SPECIAL_DELETE:
		_input_ctrl_delete(opt, ctx);
		break;
	case EOF:
		return EOF;
	default:
		break;
	}

	return 0;
}

/*
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
** Auxiliary Functions
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
int is_terminator(int ch)
{
	// It seems MOS accept Carrige as line separator...
	return ch == LINEFEED || ch == CARRIGE;
}


/*
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
** Command Execution
**+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

const char LOGO[] = "           _____                    _____                    _____                    _____          \n          /\\    \\                  /\\    \\                  /\\    \\                  /\\    \\         \n         /::\\    \\                /::\\    \\                /::\\    \\                /::\\____\\        \n        /::::\\    \\              /::::\\    \\              /::::\\    \\              /:::/    /        \n       /::::::\\    \\            /::::::\\    \\            /::::::\\    \\            /:::/    /         \n      /:::/\\:::\\    \\          /:::/\\:::\\    \\          /:::/\\:::\\    \\          /:::/    /          \n     /:::/__\\:::\\    \\        /:::/__\\:::\\    \\        /:::/__\\:::\\    \\        /:::/____/           \n    /::::\\   \\:::\\    \\      /::::\\   \\:::\\    \\       \\:::\\   \\:::\\    \\      /::::\\    \\           \n   /::::::\\   \\:::\\    \\    /::::::\\   \\:::\\    \\    ___\\:::\\   \\:::\\    \\    /::::::\\    \\   _____  \n  /:::/\\:::\\   \\:::\\    \\  /:::/\\:::\\   \\:::\\    \\  /\\   \\:::\\   \\:::\\    \\  /:::/\\:::\\    \\ /\\    \\ \n /:::/  \\:::\\   \\:::\\____\\/:::/  \\:::\\   \\:::\\____\\/::\\   \\:::\\   \\:::\\____\\/:::/  \\:::\\    /::\\____\\\n \\::/    \\:::\\  /:::/    /\\::/    \\:::\\  /:::/    /\\:::\\   \\:::\\   \\::/    /\\::/    \\:::\\  /:::/    /\n  \\/____/ \\:::\\/:::/    /  \\/____/ \\:::\\/:::/    /  \\:::\\   \\:::\\   \\/____/  \\/____/ \\:::\\/:::/    / \n           \\::::::/    /            \\::::::/    /    \\:::\\   \\:::\\    \\               \\::::::/    /  \n            \\::::/    /              \\::::/    /      \\:::\\   \\:::\\____\\               \\::::/    /   \n             \\::/    /               /:::/    /        \\:::\\  /:::/    /               /:::/    /    \n              \\/____/               /:::/    /          \\:::\\/:::/    /               /:::/    /     \n                                   /:::/    /            \\::::::/    /               /:::/    /      \n                                  /:::/    /              \\::::/    /               /:::/    /       \n                                  \\::/    /                \\::/    /                \\::/    /        \n                                   \\/____/                  \\/____/                  \\/____/         \n";
const char BANNER[] = "                  C O P Y R I G H T (C)  T O N Y ' S  S T U D I O  2 0 2 0 - 2 0 2 3";
const char THANKS[] = "                                T H A N K S  F O R  Y O U R  U S I N G";

/********************************************************************
** Try execute internal command. For now, internal command does not
** support redirect or pipe. :(
** cmd is already stripped.
** return -1 means command not found, 0 means success. Other return
** value must less than -1, and indicate failure.
*/
static void _clear_screen();
static void _print_version();
static void _print_logo();

static int _cd(int argc, char* argv[]);
static int _pwd(int argc, char* argv[]);

int execute_internal(char* argv[])
{
	int argc = 0;
	while (argv[argc])
		argc++;
	if (argc == 0)
		return -1;

	if (is_the_same(argv[0], "cd"))
	{
		return _cd(argc, argv);
	}
	else if (is_the_same(argv[0], "pwd"))
	{
		return _pwd(argc, argv);
	}
	else if (is_the_same(argv[0], "clear"))
	{
		_clear_screen();
		return 0;
	}
	else if (is_the_same(argv[0], "exit"))
	{
		printfc(FOREGROUND_INTENSE(MAGENTA), "See you later~\n");
		exit();
		printfc(FOREGROUND_INTENSE(RED), "Why you're still here?\n");
	}
	else if (is_the_same(argv[0], "version"))
	{
		_print_version();
		return 0;
	}
	else if (is_the_same(argv[0], "psh"))
	{
		_print_logo();
		return 0;
	}

	return -1;
}

static void _clear_screen()
{
	printf("\033[2J");		// clear screen
	printf("\033[3J");		// history
	printf("\033[0;0H");	// go to origin
}

static void _print_version()
{
	printfc(FOREGROUND_INTENSE(CYAN), "# Pash Host for MOS Version: %s\n", "0.1.0");
}

static void _print_logo()
{
	const int COLOR[] = {
		FOREGROUND(RED),
		FOREGROUND(GREEN),
		FOREGROUND(YELLOW),
		FOREGROUND(BLUE),
		FOREGROUND(MAGENTA),
		FOREGROUND(CYAN),
		FOREGROUND_INTENSE(RED),
		FOREGROUND_INTENSE(GREEN),
		FOREGROUND_INTENSE(YELLOW),
		FOREGROUND_INTENSE(BLUE),
		FOREGROUND_INTENSE(MAGENTA),
		FOREGROUND_INTENSE(CYAN)
	};

	int color = 0;
	const int COLOR_MAX = 12;

	_clear_screen();
	
	// print version
	//_print_version();
	//printf("\n");

	// print banner
	for (const char* p = BANNER; *p; p++)
	{
		if (*p != ' ')
		{
			printfc(COLOR[color], "%c", *p);
			color = (color + 1) % COLOR_MAX;
		}
		else
		{
			printf(" ");
		}
	}
	printf("\n");

	// print logo
	for (const char* p = LOGO; *p; p++)
	{
		if (*p == ':')
		{
			printfc(COLOR[color], "%c", *p);
			color = (color + 1) % COLOR_MAX;
		}
		else
			printf("%c", *p);
	}
	printf("\n");

	// print thanks
	for (const char* p = THANKS; *p; p++)
	{
		if (*p != ' ')
		{
			printfc(COLOR[color], "%c", *p);
			color = (color + 1) % COLOR_MAX;
		}
		else
		{
			printf(" ");
		}
	}
	printf("\n");
}

static oldPath[MAXPATHLEN];

// cd
static int _cd(int argc, char* argv[])
{
	if (argc > 2)
	{
		PASH_ERR("Too many arguments\n");
		PASH_MSG("Usage: cd [dir]\n");
		return -2;
	}

	if ((argc == 2) && is_the_same(argv[1], "-"))
	{
		if (*oldPath)
		{
			char temp[MAXPATHLEN];
			getcwd(temp);
			chdir(oldPath);
			strcpy(oldPath, temp);
		}
		else
			printfc(ERROR_COLOR, "No previous directory\n");
		return 0;
	}

	getcwd(oldPath);

	if (argc == 1)
	{
		chdir("/home");
		return 0;
	}
	if (is_the_same(argv[1], "~"))
	{
		chdir("/home");
		return 0;
	}
	else if (is_the_same(argv[1], "/"))
	{
		chdir("/");
		return 0;
	}

	if (!access(argv[1], FTYPE_DIR))
	{
		PASH_ERR("No such file or directory\n");
		return -3;
	}

	char dir[MAXPATHLEN];
	int ret = fullpath(argv[1], dir);
	if (ret != 0)
	{
		printfc(ERROR_COLOR, "fullpath: %d\n", ret);
		return ret;
	}

	// debugf("fullpath result: %s\n", dir);

	if (is_the_same(dir, ""))
		return chdir("/");

	return chdir(dir);
}

static int _pwd(int argc, char* argv[])
{
	char path[MAXPATHLEN];

	getcwd(path);

	printf("%s\n", path);

	return 0;
}
