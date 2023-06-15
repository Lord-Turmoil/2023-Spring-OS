#include <types.h>

void* memcpy(void* dst, const void* src, size_t n)
{
	void* dstaddr = dst;
	void* max = dst + n;

	if (((u_long)src & 3) != ((u_long)dst & 3))
	{
		while (dst < max)
		{
			*(char*)dst++ = *(char*)src++;
		}
		return dstaddr;
	}

	while (((u_long)dst & 3) && dst < max)
	{
		*(char*)dst++ = *(char*)src++;
	}

	// copy machine words while possible
	while (dst + 4 <= max)
	{
		*(uint32_t*)dst = *(uint32_t*)src;
		dst += 4;
		src += 4;
	}

	// finish the remaining 0-3 bytes
	while (dst < max)
	{
		*(char*)dst++ = *(char*)src++;
	}
	return dstaddr;
}

void* memset(void* dst, int c, size_t n)
{
	void* dstaddr = dst;
	void* max = dst + n;
	u_char byte = c & 0xff;
	uint32_t word = byte | byte << 8 | byte << 16 | byte << 24;

	while (((u_long)dst & 3) && dst < max)
	{
		*(u_char*)dst++ = byte;
	}

	// fill machine words while possible
	while (dst + 4 <= max)
	{
		*(uint32_t*)dst = word;
		dst += 4;
	}

	// finish the remaining 0-3 bytes
	while (dst < max)
	{
		*(u_char*)dst++ = byte;
	}
	return dstaddr;
}

size_t strlen(const char* s)
{
	int n;

	for (n = 0; *s; s++)
	{
		n++;
	}

	return n;
}

/********************************************************************
** Who implemented this? ****
*/
char* strcpy(char* dst, const char* src)
{
	char* ret = dst;

	for (const char* p = src; *p; p++)
		*(dst++) = *p;
	*dst = '\0';

	return ret;
}

const char* strchr(const char* s, int c)
{
	for (; *s; s++)
	{
		if (*s == c)
		{
			return s;
		}
	}
	return 0;
}

// Simple O(n^2) algorithm.
const char* strstr(const char* src, const char* pattern)
{
	const char* base = src;
	const char* p1 = base;
	const char* p2 = pattern;

	while (*p1 && *p2)
	{
		if (*p1 == *p2)
		{
			p1++;
			p2++;
		}
		else
		{
			p1 = ++base;
			p2 = pattern;
		}
	}

	if (*p2)
		return NULL;
	return base;
}

int strcmp(const char* p, const char* q)
{
	while (*p && *p == *q)
	{
		p++, q++;
	}

	if ((u_int)*p < (u_int)*q)
	{
		return -1;
	}

	if ((u_int)*p > (u_int)*q)
	{
		return 1;
	}

	return 0;
}

char* strcat(char* dst, const char* src)
{
	char* p = dst;

	while (*p)
		p++;
	while (*src)
		*(p++) = *(src++);
	*p = '\0';

	return dst;
}

char* strstrip(char* str, int c)
{
	if (!str)
		return NULL;

	char* base = str;
	char* left = base;
	char* right = base + strlen(str) - 1;

	while ((left <= right) && (*left == c))
		left++;
	while ((left <= right) && (*right == c))
		right--;

	for (char* p = left; p <= right; p++)
		*(base++) = *p;
	*base = '\0';

	return str;
}

char* strstripr(char* str, int c)
{
	if (!str)
		return NULL;

	char* base = str;
	char* left = base;
	char* right = base + strlen(str) - 1;

	while ((left <= right) && (*right == c))
	{
		*right = '\0';
		right--;
	}

	return str;
}

char* strstripl(char* str, int c)
{
	if (!str)
		return NULL;

	char* base = str;
	char* left = base;
	char* right = base + strlen(str) - 1;

	while ((left <= right) && (*left == c))
		left++;

	for (char* p = left; p <= right; p++)
		*(base++) = *p;
	*base = '\0';

	return str;
}
