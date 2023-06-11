#include <lib.h>
#include <print.h>

struct print_ctx
{
	int fd;
	int ret;
};

struct sprint_ctx
{
	char* buf;
	size_t cnt;
};

static void print_output(void* data, const char* s, size_t l)
{
	struct print_ctx* ctx = (struct print_ctx*)data;
	if (ctx->ret < 0)
	{
		return;
	}
	int r = write(ctx->fd, s, l);
	if (r < 0)
	{
		ctx->ret = r;
	}
	else
	{
		ctx->ret += r;
	}
}

static void soutput(void* data, const char* buf, size_t len)
{
	struct sprint_ctx* ctx = (struct sprint_ctx*)data;
	for (size_t i = 0; i < len; i++)
		ctx->buf[ctx->cnt++] = buf[i];
}

static int vfprintf(int fd, const char* fmt, va_list ap)
{
	struct print_ctx ctx;
	ctx.fd = fd;
	ctx.ret = 0;
	vprintfmt(print_output, &ctx, fmt, ap);
	return ctx.ret;
}

int fprintf(int fd, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int r = vfprintf(fd, fmt, ap);
	va_end(ap);
	return r;
}

int sprintf(char* buffer, const char* fmt, ...)
{
	va_list ap;
	struct sprint_ctx ctx;

	ctx.buf = buffer;
	ctx.cnt = 0;

	va_start(ap, fmt);
	vprintfmt(soutput, (void*)(&ctx), fmt, ap);
	va_end(ap);

	ctx.buf[ctx.cnt] = '\0';

	return (int)ctx.cnt;
}

int printf(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int r = vfprintf(1, fmt, ap);
	va_end(ap);
	return r;
}

int printfc(int color, const char* fmt, ...)
{
	// set color
	printf("\033[%dm", color);

	va_list ap;
	va_start(ap, fmt);
	int r = vfprintf(1, fmt, ap);
	va_end(ap);

	// reset color
	printf("\033[0m");

	return r;
}
